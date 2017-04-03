#if defined(OS390)
#pragma nomargins
#pragma nosequence
#endif

/***********************************************************************
*
* FOCAL - A FOCAL language interpreter
*
* Copyright (c) 1978-2011 Dave Pitts <dpitts@cozx.com>
*
* FOCAL is language developed by Digital Equipment Corporation (DEC).
* To learn FOCAL syntax refer to DEC Programming Languages Handbook:
*
* http://www.bitsavers.org/pdf/dec/pdp8/handbooks/programmingLanguages_May70.pdf
*
*
* FOCAL is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free
* Software Foundation; either version 2, or (at your option) any later
* version.
*
* FOCAL is distributed in the hope that it will be useful, but WITHOUT ANY
* WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License
* along with FOCAL; see the file COPYING.  If not, write to the Free
* Software Foundation, 59 Temple Place - Suite 330, Boston, MA
* 02111-1307, USA.
*
***********************************************************************/

#define VERSION "1.0.5"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#if defined(BSDDIR) || defined(SYSVDIR)
#include <sys/types.h>
#include <sys/stat.h>
#if defined(BSDDIR)
#include <sys/dir.h>
#endif
#if defined(SYSVDIR)
#include <dirent.h>
#endif
#endif

#if defined(OS390)
#include <stdlib.h>
#endif
 
#if defined(VAXVMS)
#include <stsdef.h>
#include <ssdef.h>
#define NORMAL          (SS$_NORMAL|STS$M_INHIB_MSG)
#define ABORT           (SS$_ABORT|STS$M_INHIB_MSG)
#define STDERROUT	"\n\n"
#else /* VAXVMS */
#define NORMAL          0
#define ABORT           16
#define STDERROUT	"\n"
#endif /* VAXVMS */

#if defined(DX10)
#include <sys/svc.h>
#endif

#include "parser.h"
#include "errors.h"
 
#define EOL       '\0'                  /* end of line */
#define TWOCHAR   2                     /* group/step sizes */
#define PLEN      132                   /* print buffer length */
#define PBEG      2                     /* print buffer start */

#ifdef DX10
#define LINE_LEN  80                    /* source line length */
#define PATH_LEN  255                   /* path name length */
#else
#define LINE_LEN  132                   /* source line length */
#define PATH_LEN  1024                  /* path name length */
#endif
 
#define DO_FLG    1
#define FOR_FLG   2
 
#ifndef TRUE
#define TRUE 1
#endif
 
#ifndef FALSE
#define FALSE 0
#endif
 
#ifndef NULL
#define NULL (void *)0
#else
#undef  NULL
#define NULL (void *)0
#endif

#define FOCAL_EXT ".foc"
#define FOCAL_LST ".lst"

 
/* Symbol node */
typedef struct sym_node
{
   struct sym_node *sym_ptr;
   tokval           sym_value;
   int              sym_index;
   char             symbol[TWOCHAR+1];
} sym_node_t;
 
/* Line node */
typedef struct line_node
{
   struct line_node *line_ptr;
   char              grp_num[TWOCHAR+1];
   char              stp_num[TWOCHAR+1];
   char              line_txt[LINE_LEN+1];
} line_node_t;
 
/* PC stack element */
typedef struct pc_stk
{
   struct pc_stk *pc_ptr;
   char          *pc_index;
   line_node_t   *old_pc;
   int            pc_flags; /* DO_FLG, FOR_FLG */
} pc_stk_t;
 
/* FOR loop stack element */
typedef struct for_stk
{
   struct for_stk *for_ptr;
   tokval          for_inc;
   tokval          for_limit;
   tokval          for_subscript;
   char            for_index[TWOCHAR+1];
} for_stk_t;
 
/* DO stack element */
typedef struct do_stk
{
   struct do_stk *do_ptr;
   int            do_flag;
   char           do_grp[TWOCHAR+1];
} do_stk_t;
 
/* Static data */
 
static line_node_t *buffer;             /* pointer to buffer */
static line_node_t *line_anchor;        /* text lines anchor */
static line_node_t *pc;                 /* the PC */
static sym_node_t  *sym_anchor;         /* symbol table anchor */
static pc_stk_t    *pc_top;             /* top of the PC stack */
static for_stk_t   *for_top;            /* top of the FOR loop stack */
static do_stk_t    *do_top;             /* top of the DO routine stack */
#ifndef SMALL_MEMORY
static line_node_t *free_line;          /* free lines */
static sym_node_t  *free_sym;           /* free symbols */
static for_stk_t   *free_for;           /* free for */
static pc_stk_t    *free_pc;            /* free pcs */
static do_stk_t    *free_do;            /* free dos */
#endif
 
static int run_mode;                    /* program running mode 'GO' */
static int do_mode;                     /* program in subroutine 'DO' */
static int quit_flag;                   /* terminate FOCAL or program */
static int trace_flag;			/* trace program execution */
static int user_stop;                   /* user keyboard interrupt */
 
static int pndx;                        /* index into print buffer */
static int width;                       /* print field width */
static int digits;                      /* print field significance */
static long seed;                       /* random number seed */
 
static char *pcp;                       /* pointer into current PC line */
static char tbuf[LINE_LEN+1];
static char wd[PATH_LEN+1];
 
static FILE *input;
static FILE *output;
static FILE *progfile;
 
/* Function prototypes */
 
static void askcmd (void);
static void cleaner (void);
static void continuecmd (void);
static void docmd (void);
static void erasecmd (void);
static void execline (void);
static tokval expression (void);
static line_node_t *findline (void);
static void forcmd (void);
static void getgrp (char *);
static void getstp (char *);
static void gocmd (void);
static void gotoit (void);
static void getsym (char *);
static void helpcmd (void);
static void ifcmd (void);
static void insertline (void);
static void librarycmd (void);
static void modifycmd (void);
static void nextfield (void);
static void pcpop (void);
static void pcpush (void);
static void quitcmd (void);
static void returncmd (void);
static void setcmd (void);
static void typecmd (void);
static void writecmd (void);
 
extern long time (long *);

void error (int, int);
void symboltable (char *, tokval *, int, int);
extern tokval Parser (char *);

#if defined(ANSICRT)
extern void clearline (void);
extern void clearscreen (void);
extern void screenposition (char *, char*);
#endif
 
#if defined(OS390)
#define err_flag ERRFLAG
#endif
int err_flag;
int fpe_stop;                           /* floating point exception */

/* some toupper functions don't check case first */
#ifndef NOFOLD
#define upcase toupper
#endif

#ifdef DEBUG_FILE
/***********************************************************************
* HEXDUMP
***********************************************************************/

static void
HEXDUMP (FILE *file, void *ptr, int size)
{
   int jjj;
   int iii;
   char *tp;
   char *cp;
   for (iii = 0, tp = (char *)(ptr), cp = (char *)(ptr); iii < (size); )
   {
      fprintf ((file), "%04x  ", iii);
      for (jjj = 0; jjj < 8; jjj++)
      {
	 if (cp < ((char *)(ptr)+(size)))
	 {
	    fprintf ((file), "%02.2x", *cp++ & 0xFF);
	    if (cp < ((char *)(ptr)+(size)))
	    {
	       fprintf ((file), "%02.2x ", *cp++ & 0xFF);
	    }
	    else
	    {
	       fprintf ((file), "   ");
	    }
	 }
	 else
	 {
	    fprintf ((file), "     ");
	 }
	 iii += 2;
      }
      fprintf ((file), "   ");
      for (jjj = 0; jjj < 8; jjj++)
      {
	 if (tp < ((char *)(ptr)+(size)))
	 {
	    if (isprint(*tp))
	       fprintf ((file), "%c", *tp);
	    else
	       fprintf ((file), ".");
	    tp++;
	    if (tp < ((char *)(ptr)+(size)))
	    {
	       if (isprint(*tp))
		  fprintf ((file), "%c ", *tp);
	       else
		  fprintf ((file), ". ");
	       tp++;
	    }
	    else
	    {
	       fprintf ((file), "  ");
	    }
	 }
	 else
	 {
	    fprintf ((file), "   ");
	 }
      }
      fprintf ((file), "\n");
   }
}
#endif

#if defined(DX10)
/***********************************************************************
* checkterm = check for terminal event.
***********************************************************************/

void
checkterm (void)
{
   SVCEvtBlk svc;

   svc.opcode = 0x39;
   svc.status = 0;
   svc.event = 0;
   svc.luno = 0;

   _issue_svc ((SVCBlk *)&svc);
   if (svc.status == 0 && svc.event == 0x98)
      user_stop = TRUE;
}
#endif

#ifdef NOFOLD
/***********************************************************************
* upcase
***********************************************************************/

char
upcase (char ch)
{
   if (islower (ch))
      ch = toupper (ch);
   return (ch);
}
#endif
 
/***********************************************************************
* freedo - free a DO element
***********************************************************************/
 
static void
freedo (do_stk_t *p)
{
#ifdef SMALL_MEMORY
   free (p);
#else
   p->do_ptr = free_do;
   free_do = p;
#endif
} /* freedo */
 
/***********************************************************************
* freefor - free a FOR element
***********************************************************************/
 
static void
freefor (for_stk_t *p)
{
#ifdef SMALL_MEMORY
   free (p);
#else
   p->for_ptr = free_for;
   free_for = p;
#endif
} /* freefor */
 
/***********************************************************************
* freeline - free a line element
***********************************************************************/
 
static void
freeline (line_node_t *p)
{
#ifdef SMALL_MEMORY
   free (p);
#else
   p->line_ptr = free_line;
   free_line = p;
#endif
} /* freeline */
 
/***********************************************************************
* freesym - free a sym element
***********************************************************************/
 
static void
freesym (sym_node_t *p)
{
#ifdef SMALL_MEMORY
   free (p);
#else
   p->sym_ptr = free_sym;
   free_sym = p;
#endif
} /* freesym */
 
/************************************************************************
*
* ASK - ask for user input
*
* Procedure ASK processes the ASK command. The forms recognized are
* as follows:
*       A(SK) <var>              ask for a variable
*       A(SK) "PROMPT",<var>     ask for a variable w/prompting
*
************************************************************************/

static void
askcmd (void)
{
   tokval val;
   int ndx;
   int j;
   char delim;
   char row[TWOCHAR+1];
   char col[TWOCHAR+1];
   char sym[TWOCHAR+1];
 
   /* Position to the next field */
   nextfield();
   fflush (output);
 
   do
   {
 
      /* Error if function  */
      if (upcase (*pcp) == 'F')
         error (BAD_FUNC, 0);
 
      /* Ask the value for input symbol */
      else if (isalpha (*pcp))
      {
         getsym (sym);
         ndx = 0;
         if (*pcp == '(' || *pcp == '[' || *pcp == '<' || *pcp == '{')
         {
            /* Get subscript */
            ndx = expression ();
         }
 
         /* Prompt for input */
 
         fputs (":", stdout);
#if defined(OS390) || defined(OS2)
	 fputc ('\n', stdout);
#endif
#if defined(OS390) || defined(OS2) || defined(DX10)
         fflush (stdout);
#endif
         pndx = PBEG;
 
         /* Read input */
         if ((fgets (tbuf, sizeof (tbuf), stdin) != NULL) && (tbuf[0] != '\n'))
            val = Parser (tbuf);
         else
            val = 0.0;
 
         /* Save in symbol table */
         symboltable (sym, &val, ndx, FALSE);
      }
 
      /* check for new line */
      else if (*pcp == '!')
      {
         pcp++;
         fputc ('\n', stdout);
         pndx = PBEG;
      }
 
      /* check for prompt text */
      else if (*pcp == '"' || *pcp == '\'')
      {
         delim = *pcp++;
	 if (trace_flag)
	 {
	    fputc (delim, output);
	 }
         while (*pcp != EOL && *pcp != delim)
            fputc (*pcp++, stdout);
         if (*pcp == EOL)
            error (BAD_STRING, 0);
         else
            pcp++;
	 if (trace_flag)
	 {
	    fputc (delim, output);
	 }
      }
 
#if defined(ANSICRT)
      /* Position on CRT */
      else if (*pcp == '@')
      {
         pcp++;
         j = 0;
         strcpy (row, "01");
         strcpy (col, "01");
         if (upcase (*pcp) == 'E')
         {
            pcp++;
            clearscreen();
         }
         if (isdigit (*pcp))
         {
            getgrp (row);
            if (*pcp == '.')
            {
               pcp++;
               getgrp (col);
            }
            screenposition (row, col);
         }
         if (upcase (*pcp) == 'C')
         {
            pcp++;
            clearline();
         }
      }
#endif
 
      else if (*pcp == EOL || *pcp == ';') ;
 
      else pcp++;
 
   } while (!err_flag && *pcp && *pcp != EOL && *pcp != ';');
 
} /* ask */
 
/************************************************************************
*
* CLEANER - Clean up after error
*
* This procedure cleans up after an error.
*
************************************************************************/

static void
cleaner (void)
{
   do_stk_t  *p;
   for_stk_t *q;
 
   /* Write line number */
   if (run_mode)
      fprintf (stderr, " @ %s.%s", pc->grp_num, pc->stp_num);
 
   /* Purge PC stack */
   while (pc_top != NULL) pcpop();
 
   /* Purge DO stack */
   while (do_top != NULL)
   {
      p = do_top;
      do_top = p->do_ptr;
      freedo (p);
   }
 
   /* Purge FOR stack */
   while (for_top != NULL)
   {
      q = for_top;
      for_top = q->for_ptr;
      freefor (q);
   }
 
   run_mode = FALSE;
   do_mode = FALSE;
 
   /* print error message */
   fprintf (stderr, STDERROUT);
 
   if (output != stdout)
      fputs ("Errors in program execution\n", stdout);
 
} /* cleaner */
 
/************************************************************************
*
* CONTINUECMD - Continue/comment
*
* Procedure CONTINUE processes the CONTINUE/COMMENT command.
*
************************************************************************/

static void
continuecmd (void)
{
 
   while (*pcp && *pcp != ';' && *pcp != EOL) pcp++;
 
} /* continuecmd */
 
/************************************************************************
*
* DOCMD - DO subroutine command
*
* Procedure DOCMD process the DO command. The syntax is:
*        D(O) <GRP>[.<STP>]
*
************************************************************************/

static void
docmd (void)
{
   do_stk_t *p;
   line_node_t *l;
   int found;
   char sym[TWOCHAR+1];
 
   /* Position to group field */
   nextfield();
   if (*pcp == '?')
   {
      trace_flag = TRUE;
      nextfield();
   }
 
#ifdef SMALL_MEMORY
   if ((p = (do_stk_t *)malloc (sizeof (do_stk_t))) == NULL)
   {
      error (MEM_OVERFLOW, STACK_OVERFLOW);
   }
#else
   if (free_do == NULL)
   {
      if ((p = (do_stk_t *)malloc (sizeof (do_stk_t))) == NULL)
      {
         error (MEM_OVERFLOW, STACK_OVERFLOW);
      }
   }
   else
   {
      p = free_do;
      free_do = p->do_ptr;
   }
#endif
 
   if (p != NULL)
   {
 
      memset ((void *)p, 0, sizeof (do_stk_t));

      /* Get group from buffer */
      getgrp (p->do_grp);
      p->do_flag = TRUE;
 
      /* Find group in line list */
      l = line_anchor;
      found = FALSE;
      while (l != NULL && !found)
         if (!strcmp (l->grp_num, p->do_grp))
            found = TRUE;
         else
            l = l->line_ptr;
 
      if (l != NULL)
      {
         /* Check for step */
         if (*pcp == '.')
         {
            pcp++;
            p->do_flag = FALSE;
            getstp (sym); /* get step */
            if (!err_flag)
            {
               found = FALSE;
               while (l != NULL && !found)
                  if (!strcmp (p->do_grp, l->grp_num) &&
                      !strcmp (sym, l->stp_num))
                     found = TRUE;
                  else
                     l = l->line_ptr;
               if (l == NULL)
               {
                  error (BAD_LINE, DO_TARGET); /* bad line number */
                  freedo (p);
               }
            }
         }
 
         nextfield();
         if (!err_flag)
         {
            /* Push where we are */
            pcpush();
            pc_top->pc_flags = DO_FLG;
            pc = l;
            pcp = pc->line_txt;
	    if (trace_flag)
	    {
	       fprintf (stdout, "\n+D%s.%s ", pc->grp_num, pc->stp_num);
	    }
            /* Set up DO stack */
            p->do_ptr = do_top;
            do_top = p;
            do_mode = TRUE;
         }
      }
 
      else
      {
         error (BAD_LINE, DO_TARGET);
         freedo (p);
      }
 
   }
 
} /* docmd */
 
/************************************************************************
*
* ERASECMD - Erase lines and symbols
*
* Procedure ERASE deletes the symbol table and program lines.
*
************************************************************************/

static void
erasecmd (void)
{
   sym_node_t *p, *p1;
   line_node_t *q, *q1;
   int found;
   int k;
   char grp[TWOCHAR+1], stp[TWOCHAR+1];
 
   nextfield();
   k = 2;
 
   /* Erase symbol table */
   if (*pcp == EOL || *pcp == ';')
      k = 0;
 
   /* Erase program and symbols */
   else if (upcase (*pcp) == 'A')
   {
      nextfield();
      k = 0;
      q = line_anchor;
      line_anchor = NULL;
      while (q != NULL)
      {
         q1 = q->line_ptr;
         freeline (q);
         q = q1;
      }
   }
 
   /* Erase specified lines */
   else if (isdigit (*pcp))
   {
      k = 2;
      getgrp (grp);
      q = line_anchor;
      q1 = line_anchor;
      found = FALSE;
 
      while (q != NULL && !found)
         if (!strcmp (q->grp_num, grp))
            found = TRUE;
         else
         {
            q1 = q;
            q = q->line_ptr;
         }
 
      if (q != NULL)
      {
         if (*pcp == '.')
         {
            pcp++;
            getstp (stp);
            found = FALSE;
            if (!err_flag)
            {
               while (q != NULL && !found)
                  if (!strcmp (q->grp_num, grp) &&
                      !strcmp (q->stp_num, stp))
                  {
                     found = TRUE;
                  }
                  else
                  {
                     q1 = q;
                     q = q->line_ptr;
                  }
            }
            if (q != NULL)
            {
               if (q != line_anchor)
               {
                  q1->line_ptr = q->line_ptr;
                  freeline (q);
               }
               else
               {
                  line_anchor = q->line_ptr;
                  freeline (q);
               }
            }
         }
 
         else
         {
            found = FALSE;
            if (q != line_anchor)
            {
               while (q != NULL && !found)
                  if (!strcmp (q->grp_num, grp))
                  {
                     q1->line_ptr = q->line_ptr;
                     freeline (q);
                     q = q1->line_ptr;
                  }
                  else found = TRUE;
            }
            else while (q != NULL && !found)
            {
                  if (!strcmp (q->grp_num, grp))
                  {
                     line_anchor = q->line_ptr;
                     freeline (q);
                     q = line_anchor;
                  }
                  else
                     found = TRUE;
            }
         }
 
      }
   }
 
   else
      error (BAD_LINE, 0);
 
   /* Erase symbols */
   if (k == 0)
   {
      p = sym_anchor;
      sym_anchor = NULL;
      while (p != NULL)
      {
         p1 = p->sym_ptr;
         freesym (p);
         p = p1;
      }
   }
 
} /* erasecmd */
 
/************************************************************************
*
* ERROR - General error processor
*
* Procedure ERROR genertates error messages from a passed error code
*
************************************************************************/

void
error (int err_code, int err_stat)
{
   char errorstring[256];
 
   err_flag = TRUE;
   errorstring[0] = '\0';
 
   /* Print pending text */
   if (pndx > PBEG)
   {
      fputc ('\n', output);
      pndx = PBEG;
   }
 
   switch (err_code)
   {
 
      case USER_STOP:
         user_stop = FALSE;
         strcpy (errorstring, "Stop");
         break;
      
#if !defined(DX10)
      case FPE_STOP:
         fpe_stop = FALSE;
         strcpy (errorstring, "Floating point exception");
         break;
#endif
 
      case BAD_CMD:
         strcpy (errorstring, "Bad command");
         if (err_stat == LIBRARY_CMD)
	    strcat (errorstring, " in Library");
	 strcat (errorstring, "\nType HELP for a list of commands");
         break;
 
      case BAD_LINE:
         strcpy (errorstring, "Bad line number");
         switch (err_stat)
         {
            case GO_TARGET:
               strcat (errorstring, " in GOTO");
               break;
            case DO_TARGET:
               strcat (errorstring, " in DO");
	       break;
            default: ;
         }
         break;
 
      case BAD_VAR:
         strcpy (errorstring, "Bad variable");
         break;
 
      case BAD_EXPR:
         strcpy (errorstring, "Bad expression");
         switch (err_stat)
         {
            case FOR_EXPR :
               strcat (errorstring, " in FOR/SET, missing '='");
               break;
            case IF_EXPR:
               strcat (errorstring, " in IF");
               break;
            case TAB_EXPR:
               strcat (errorstring, " in tab value");
               break;
            default: ;
         }
         break;
 
      case UNDEF_FUNC:
         strcpy (errorstring, "Undefined function");
         break;
 
      case BAD_FUNC:
         strcpy (errorstring, "Bad function usage");
         break;
 
      case BAD_STRING:
         strcpy (errorstring, "Bad text string");
         break;
 
      case BAD_NUM:
         strcpy (errorstring, "Bad number");
         break;
 
      case MEM_OVERFLOW:
         strcpy (errorstring, "Overflow - ");
         switch (err_stat)
         {
            case LINE_OVERFLOW:
               strcat (errorstring, "line buffer");
               break;
            case SYMBOL_OVERFLOW:
               strcat (errorstring, "symbol table");
               break;
            case STACK_OVERFLOW:
               strcat (errorstring, "stack");
	       break;
            default: ;
         }
         break;
 
      case PARSE_ERROR:
         switch (err_stat)
         {
           /* Get the generated parser errors */
#include "perrors.h"
            default:
               sprintf (errorstring, "Parse error = %d", err_stat);
         }
         break;
 
      case SCAN_ERROR:
         switch (err_stat)
         {
            case 4:
            case 9:
               strcpy (errorstring, "Bad exponent sign");
               break;
            case 7:
               strcpy (errorstring, "Bad fraction");
               break;
            case 8:
               strcpy (errorstring, "Exponent overflow");
               break;
            default:
               sprintf (errorstring, "Scan error = %d", err_stat);
         }
         break;
 
      case INTERP_ERROR:
         switch (err_stat)
         {
            case ZERO_DIVIDE:
               strcpy (errorstring, "Divide by zero");
               break;
            case NEG_SQRT:
               strcpy (errorstring, "Negative SQRT");
               break;
            case NEG_LOG:
               strcpy (errorstring, "Negative or zero LOG");
               break;
            default:
               sprintf (errorstring, "Interp error = %d", err_stat);
         }
         break;
 
      case MODIFY_ERROR:
         strcpy (errorstring, "No match found");
         break;
 
      case FILE_ERROR:
         break;
 
      default:
         strcpy (errorstring, "Undefined error");
   }

   fputs (errorstring, stderr);
 
   /* clean up */
   cleaner();
 
} /* error */
 
/************************************************************************
*
* EXECLINE - Execute line
*
* Procedure EXEC_LINE processes source/command lines pointed to by
* the PC.
*
************************************************************************/

static void
execline (void)
{
   do_stk_t *q;
   for_stk_t *q1;
   line_node_t *p;
   tokval val;
   int doit, next;
 
#ifdef DEBUG_INPUT
   printf ("execline: entered\n");
#endif

   do
   {
 
      do
      {
 
         /* Process command */
	 if (trace_flag)
	 {
	    if (isalpha (*pcp))
	       fprintf (stdout, "%c, ", upcase (*pcp));
	 }
         switch (upcase (*pcp++))
         {
 
            case 'A' : askcmd();      break;
          /*case 'B' : AVAILABLE      break;*/
            case 'C' : continuecmd(); break;
            case 'D' : docmd();       break;
            case 'E' : erasecmd();    break;
            case 'F' : forcmd();      break;
            case 'G' : gocmd();       break;
            case 'H' : helpcmd();     break;
            case 'I' : ifcmd();       break;
          /*case 'J' : AVAILABLE      break;*/
          /*case 'K' : AVAILABLE      break;*/
            case 'L' : librarycmd();  break;
            case 'M' : modifycmd();   break;
          /*case 'N' : AVAILABLE      break;*/
          /*case 'O' : AVAILABLE      break;*/
          /*case 'P' : AVAILABLE      break;*/
            case 'Q' : quitcmd();     break;
            case 'R' : returncmd();   break;
            case 'S' : setcmd();      break;
            case 'T' : typecmd();     break;
          /*case 'U' : AVAILABLE      break;*/
          /*case 'V' : AVAILABLE      break;*/
            case 'W' : writecmd();    break;
          /*case 'X' : AVAILABLE      break;*/
          /*case 'Y' : AVAILABLE      break;*/
          /*case 'Z' : AVAILABLE      break;*/
            case EOL : pcp--;         break;
            case ' ' :
            case ';' :                break;
            default:

	       if (trace_flag)
	       {
		  pndx = PBEG;
		  pcp--;
		  printf ("\npc = %p, pcp =%p\n", pc, pcp);
		  printf ("grp = %s, stp = %s\n", pc->grp_num, pc->stp_num);
		  printf ("line = '%s'\n", pc->line_txt);
		  printf ("%c(%02x): ", *pcp, *pcp);
	       }
               error (BAD_CMD, COMMAND);
         }
 
#if !defined(DX10)
         /* Check for floating point exception */
         if (fpe_stop)
            error (FPE_STOP, 0);
#endif
 
         /* Check for user abort */
#if defined(DX10)
         checkterm();
#endif
         if (user_stop)
            error (USER_STOP, 0);
 
      } while (*pcp && *pcp != EOL && !err_flag);
 
      next = TRUE;
 
      /* Check for loop in progress */
      if (pc_top != NULL)
      {
         if (pc_top->pc_flags == FOR_FLG)
         {
            symboltable (for_top->for_index, &val,
			 for_top->for_subscript, TRUE);
            val = val + for_top->for_inc;
            symboltable (for_top->for_index, &val,
			 for_top->for_subscript, FALSE);
            if (val <= for_top->for_limit)
            {
               next = FALSE;
               pc = pc_top->old_pc;
               pcp = pc_top->pc_index;
	       if (trace_flag)
	       {
		  fprintf (stdout, "\n-F%s.%s ", pc->grp_num, pc->stp_num);
	       }
            }
            else
            {
               pcpop();
               q1 = for_top;
               for_top = q1->for_ptr;
               freefor (q1);
               break;
            }
         }
 
         /* Not a loop must be DO */
         else
         {
            p = pc->line_ptr;
            doit = FALSE;
            if (p == NULL)
               doit = TRUE;
            else
            {
               if (strcmp (p->grp_num, do_top->do_grp))
                  doit = TRUE;
            }
            if (!do_top->do_flag)
               doit = TRUE;
            if (doit)
            {
               pc = pc_top->old_pc;
               pcp = pc_top->pc_index;
	       if (trace_flag)
	       {
		  fprintf (stdout, "\n-D%s.%s ", pc->grp_num, pc->stp_num);
	       }
               next = FALSE;
               pcpop();
               q = do_top;
               do_top = q->do_ptr;
               freedo (q);
               if (do_top == NULL)
                  do_mode = FALSE;
            }
         }
      }
 
      if ((run_mode || do_mode) && next)
      {
         /* Point PC to next line */
         p = pc->line_ptr;
         if (p != NULL)
         {
            pc = p;
            pcp = pc->line_txt;
            if (trace_flag)
            {
               fprintf (stdout, "\nN%s.%s ", pc->grp_num, pc->stp_num);
            }
         }
         else run_mode = FALSE;
      }
 
   } while (run_mode || pc_top != NULL);
 
} /* execline */
 
/************************************************************************
*
* EXPRESSION - Scan out expression
*
* Procedure EXPERSSION scans out expressions and calls the parser to
* reduce the expression to a value.
*
************************************************************************/

static tokval
expression (void)
{
   char *bp;
   char expr[LINE_LEN+1];
 
   bp = expr;
   while (*pcp && *pcp != EOL && *pcp != ',' && *pcp != ';' && *pcp != '=' &&
           *pcp != '"' && *pcp != '%' && *pcp != '!')
   {
      if (*pcp != ' ')
         *bp++ = *pcp;
      pcp++;
   }
   *bp++ = EOL;
   *bp = EOL;
   if (trace_flag)
   {
      fprintf (stdout, "expr(%s) ", expr);
   }
 
   return (Parser (expr));
 
} /* expression */
 
/************************************************************************
*
* FINDLINE - Find line
*
* Procedure FINDLINE returns the address of the line addressed by the
* line number in the buffer.
*
************************************************************************/

static line_node_t *
findline (void)
{
   line_node_t *l, *p;
   int found;
   char stp[TWOCHAR+1], grp[TWOCHAR+1];
 
   p = NULL;
 
   /* Get line group */
   getgrp (grp);
   if (*pcp == '.')
   {
      pcp++;
 
      /* Get line step */
      getstp (stp);
      p = line_anchor;
 
      /* See if going forward */
      if (run_mode)
      {
         l = pc;
         if (strcmp (l->grp_num, grp) <= 0 &&
             (strcmp (l->grp_num, grp) == 0 && strcmp (l->stp_num, stp) <= 0))
            p = l;
      }
 
      found = FALSE;
      while (p != NULL && !found)
         if (!strcmp (p->grp_num, grp) &&
	     !strcmp (p->stp_num, stp))
            found = TRUE;
         else
            p = p->line_ptr;
   }
 
   return (p);
 
} /* findline */
 
/************************************************************************
*
* FMTNUM - Format numbers
*
* This routine format numbers for printing in either tokvaling or fixed
* format.
*
************************************************************************/

static void
fmtnum (tokval val)
{
   char format[30];
 
   if (width > 0)
   {
      pndx += width;
      sprintf (format, "= %%%d.%df", width, digits);
      fprintf (output, format, val);
   }
 
   else
   {
      pndx += 13;
      fprintf (output, "= %13.6e", val);
   }
 
} /* fmtnum */
 
/************************************************************************
*
* FORCMD - FOR loop command
*
* Procedure FORCMD process the FOR statement. Syntax is:
*   F(OR) <NDX>=<START>[,<INC>],<END>;<STMT>
*
************************************************************************/

static void
forcmd (void)
{
   for_stk_t *p;
   tokval v1, val;
   int ndx;
 
   /* Skip to index variable */
   nextfield();
 
   /* Error if function  */
   if (upcase (*pcp) == 'F')
   {
      error (BAD_FUNC, 0);
      return;
   }
 
#ifdef SMALL_MEMORY
   if ((p = (for_stk_t *)malloc (sizeof (for_stk_t))) == NULL)
   {
      error (MEM_OVERFLOW, STACK_OVERFLOW);
   }
#else
   if (free_for == NULL)
   {
      if ((p = (for_stk_t *)malloc (sizeof (for_stk_t))) == NULL)
      {
         error (MEM_OVERFLOW, STACK_OVERFLOW);
      }
   }
   else
   {
      p = free_for;
      free_for = p->for_ptr;
   }
#endif
 
   if (p != NULL)
   {
 
      memset ((void *)p, 0, sizeof (for_stk_t));

      /* Get index symbol */
      getsym (p->for_index);
 
      if (*pcp == ' ')
         nextfield();

      ndx = 0;
      if (*pcp == '(' || *pcp == '<' || *pcp == '[' || *pcp == '{')
      {
	 ndx = expression();
	 if (*pcp == ' ')
	    nextfield();
      }
      p->for_subscript = ndx;
 
      if (*pcp == '=')
      {
 
         /* get start expression */
         pcp++;
         v1 = val = expression();
         symboltable (p->for_index, &val, p->for_subscript, FALSE);
 
         /* Get increment */
         pcp++;
         p->for_inc = expression();
 
         /* If EOL then this is limit and inc is 1 */
         if (*pcp == EOL || *pcp == ';')
         {
            p->for_limit = p->for_inc;
            p->for_inc = 1.0;
            if (*pcp == ';')
	       pcp++;
         }
 
         /* Get limit */
         else
         {
            pcp++;
            p->for_limit = expression();
         }
 
         if (!err_flag)
         {
            /* Set up FOR stack */
            p->for_ptr = for_top;
            for_top = p;
            /* Set up PC stack */
            pcpush();
            pc_top->pc_flags = FOR_FLG;
            /* Execute loop */
            execline();
         }
 
         else
         {
            freefor (p);
            while (*pcp && *pcp != EOL) pcp++;
         }
 
      }
 
      /* bad loop expression */
      else
      {
         error (BAD_EXPR, FOR_EXPR);
         freefor (p);
      }
 
   }
 
} /* forcmd */
 
/************************************************************************
*
* GETGRP - Get line group
*
* Procedure GET_GRP gets the group number, 1 or 2 digits, from the
* buffer and returns the value.
*
************************************************************************/

static void
getgrp (char grp[])
{
   char ch;
 
   strcpy (grp, "00");
   ch = *pcp++;
   if (isdigit (ch))
   {
      if (isdigit (*pcp))
      {
         grp[0] = ch;
         grp[1] = *pcp++;
      }
      else
         grp[1] = ch;
   }
   else
      error (BAD_LINE, 0);
 
} /* getgrp */
 
/************************************************************************
*
* GETSTP - Get line step
*
* Procedure GET_STP get the step number, 1 or 2 digits, from the buffer
* and returns the value.
*
************************************************************************/

static void
getstp (char stp[])
{
   char ch;
 
   strcpy (stp, "00");
   ch = *pcp++;
   if (isdigit (ch))
   {
      if (isdigit (*pcp))
      {
         stp[0] = ch;
         stp[1] = *pcp++;
      }
      else
         stp[0] = ch;
   }
   else
      error (BAD_LINE, 0);
 
} /* getstp */
 
/************************************************************************
*
* GETSYM - Get symbol
*
* Procedure GET_SYM gets the symbol from the buffer.
*
************************************************************************/

static void
getsym (char sym[])
{
   int j;
 
   strcpy (sym, "  ");
   j = 0;
   while (*pcp && *pcp != EOL && isalnum (*pcp))
   {
      if (j < TWOCHAR)
         sym[j++] = upcase (*pcp++);
      else
         pcp++;
   }
 
} /* getsym */
 
/************************************************************************
*
* GOCMD - Go to command
*
* Procedure GOCMD process the go/goto statements. The syntax is:
*   G(O(TO)) (<LN>) if (the line number is absent) go to lowest
*                   numbered line.
*
************************************************************************/

static void
gocmd (void)
{
 
   nextfield();
   if (*pcp == '?')
   {
      trace_flag = TRUE;
      nextfield();
   }
   if (line_anchor != NULL)
      gotoit();
 
} /* gocmd */
 
/************************************************************************
*
* GOTOIT - GOTO line
*
* Procedure GOTOIT set the PC to the line number in the buffer.
*
************************************************************************/

static void
gotoit (void)
{
   line_node_t *p;
 
   /* If line number present GOTOIT */
   if (*pcp != EOL && *pcp != ';')
   {
      p = findline();
 
      /* Set PC to target line */
      if (p != NULL)
      {
         pc = p;
         run_mode = TRUE;
      }
      else
         error (BAD_LINE, GO_TARGET);
   }
 
   /* No line number use anchor */
   else
   {
      pc = line_anchor;
      if (pc != NULL)
      {
         run_mode = TRUE;
      }
   }
 
   pcp = pc->line_txt;
   if (trace_flag)
   {
      fprintf (stdout, "\nG%s.%s ", pc->grp_num, pc->stp_num);
   }
 
} /* gotoit */
 
/************************************************************************
*
* HELPCMD - help command
*
* Procedure HELPCMD process the help command. Syntax:
*   H(ELP) [command]
*
************************************************************************/

static void
helpcmd (void)
{
#if !defined(MINFOCAL)
   int all = FALSE;

   /* Position to command, if any */
   nextfield();

   if (*pcp != EOL && *pcp != ';')
   {
      switch (upcase (*pcp++))
      {

	 case '*' :
	    all = TRUE;
	 case 'A' :
	    printf ("   A[SK] [\"PROMPT\",] <VAR>\n");
	    if (!all) break;
	 case 'C' :
	    printf ("   C[ONTINUE]\n");
	    if (!all) break;
	 case 'D' :
	    printf ("   D[O] ['?'] <GROUP>[.<STEP>]\n");
	    if (!all) break;
	 case 'E' :
	    printf ("   E[RASE] [A[LL] | <GROUP>[.<STEP>]]\n"); 
	    if (!all) break;
	 case 'F' :
	    printf ("   F[OR] <VAR>=<START>[,<INC>],<END>; <COMMAND>\n");
	    if (!all) break;
	 case 'G' :
	    printf ("   G[OTO] ['?'] [<LINE>]\n");
	    if (!all) break;
	 case 'H' :
	    printf ("   H[ELP] [<COMMAND>]\n");
	    if (!all) break;
	 case 'I' :
	    printf ("   I[F] (<EXPRESSION>) <LINE> [,<LINE>[,<LINE>]]\n");
	    if (!all) break;
	 case 'L' :
	    printf ("   L[IBRARY] C[ALL] <FILENAME>\n");
#if defined(BSDDIR) || defined(SYSVDIR)
	    printf ("   L[IBRARY] L[IST] [<DIRNAME>]\n");
#endif
	    printf ("   L[IBRARY] P[RINT] <FILENAME>\n");
	    printf ("   L[IBRARY] S[AVE] <FILENAME>\n");
#if defined(BSDDIR) || defined(SYSVDIR)
	    printf ("   L[IBRARY] W[ORK] <DIRNAME>\n");
#endif
	    if (!all) break;
	 case 'M' :
	    printf ("   M[ODIFY] <LINE> /OLDPATTERN/NEWPATTERN/\n");
	    if (!all) break;
	 case 'Q' :
	    printf ("   Q[UIT]\n");
	    if (!all) break;
	 case 'R' :
	    printf ("   R[ETURN]\n");
	    if (!all) break;
	 case 'S' :
	    printf ("   S[ET] <VAR> = <EXPRESSION>\n");
	    if (!all) break;
	 case 'T' :
	    printf ("   T[YPE] [\"TEXT\",] <VAR> | <EXPRESSION>\n");
	    if (!all) break;
	 case 'W' :
	    printf ("   W[RITE] [A[LL] | <GROUP>[.<STEP>]]\n"); 
	    if (!all) break;
	 case ' ' :
	    break;
	 default:
	    error (BAD_CMD, COMMAND);
      }
      nextfield();
   }
   else
#else
   while (*pcp != EOL && *pcp != ';') pcp++;
#endif
   {
      printf ("Commands:\n");
      printf ("   ASK      CONTINUE DO       ERASE    FOR      GOTO\n");
      printf ("   HELP     IF       LIBRARY  MODIFY   QUIT     RETURN\n");
      printf ("   SET      TYPE     WRITE\n");
   }

} /* helpcmd */

/************************************************************************
*
* IFCMD - if command
*
* Procedure IFCMD process the if command. Syntax:
*   I(F) (<EXPR>) <LN>[,<LN>[,<LN>]];
*
************************************************************************/

static void
ifcmd (void)
{
   char *bp;
   tokval val;
   int k, j;
 
   /* Position to expression */
   nextfield();
 
   /* Scan out expression */
   bp = tbuf;
   if (*pcp == '(')
   {
      while (*pcp != EOL && *pcp != ','  && *pcp != ';') *bp++ = *pcp++;
      do
      {
         pcp--;
         bp--;
      } while (*(bp-1) != ')');
      *bp++ = EOL;
      *bp = EOL;
 
      /* Get value of expression */
      val = Parser (tbuf);
      if (val == 0.0)
         j = 1;
      else if (val > 0.0)
         j = 2;
      else
         j = 0;
 
      /* Go to target line number */
      k = 0;
      while (k < j)
      {
         while (*pcp != EOL && *pcp != ',' && *pcp != ';') pcp++;
         if (*pcp == ',') pcp++;
         k++;
      }
      while (*pcp == ' ') pcp++;
      if (*pcp != EOL && *pcp != ';')
         gotoit();
 
   }
 
   else
      error (BAD_EXPR, IF_EXPR);
 
} /* ifcmd */
 
/************************************************************************
*
* INSERTLINE - Insert text line
*
* Procedure INSERT_LINE takes a source line and links it into the line
* buffer in group/step order. if the line currently exists it text is
* replaced with the new text.
*
************************************************************************/

static void
insertline (void)
{
   line_node_t *p, *next, *back;
   char *bp;
   int found;
   char olinnum[6], nlinnum[6];
 
#ifdef SMALL_MEMORY
   if ((p = (line_node_t *)malloc (sizeof (line_node_t))) == NULL)
   {
      error (MEM_OVERFLOW, LINE_OVERFLOW);
   }
#else
   if (free_line == NULL)
   {
      if ((p = (line_node_t *)malloc (sizeof (line_node_t))) == NULL)
      {
         error (MEM_OVERFLOW, LINE_OVERFLOW);
      }
   }
   else
   {
      p = free_line;
      free_line = p->line_ptr;
   }
#endif
 
   if (p != NULL)
   {
      memset ((void *)p, 0, sizeof (line_node_t));

      p->line_ptr = NULL;
      /* Get line group number */
      getgrp (p->grp_num);
      if (!err_flag)
      {
         if (*pcp == '.')
         {
            pcp++;
            /* Get line step number */
            getstp (p->stp_num);
            if (!err_flag)
            {
               while (*pcp == ' ') pcp++;
               bp = p->line_txt;
               while (*pcp)
               {
                  if (*pcp != '\n' && *pcp != '\r')
                     *bp++ = *pcp;
                  pcp++;
               }
               *bp = EOL;
#ifdef DEBUG_FILE
	       printf ("p(%d) = '%s'\n", strlen(p->line_txt), p->line_txt);
#endif
               back = line_anchor;
               next = line_anchor;
 
               /* If no lines then link at top */
               if (line_anchor == NULL)
	       {
#ifdef DEBUG_FILE
		  printf ("HEAD: p = %p\n", p);
		  HEXDUMP (stdout, (char *)p, sizeof (line_node_t));
#endif
                  line_anchor = p;
	       }
 
               /* Search for place to insert */
               else
               {
                  found = FALSE;
                  sprintf (olinnum, "%s%s", p->grp_num, p->stp_num);
#ifdef DEBUG_FILE
	          printf ("olinnum = '%s'\n", olinnum);
#endif
                  while (next != NULL && !found)
                  {
                     sprintf (nlinnum, "%s%s", next->grp_num, next->stp_num);
#ifdef DEBUG_FILE
		     printf ("nlinnum = '%s'\n", nlinnum);
#endif
                     if (strcmp (nlinnum, olinnum) < 0)
                     {
                        back = next;
                        next = next->line_ptr;
                     }
                     else
                        found = TRUE;
                  }
 
                  /* Link at end of list */
                  if (next == NULL)
		  {
#ifdef DEBUG_FILE
		     printf ("END: p = %p\n", p);
		     HEXDUMP (stdout, (char *)p, sizeof (line_node_t));
#endif
                     back->line_ptr = p;
		  }
 
                  /* Insert new line */
                  else if (strcmp (nlinnum, olinnum))
                  {
#ifdef DEBUG_FILE
		     printf ("INSERT: p = %p\n", p);
		     HEXDUMP (stdout, (char *)p, sizeof (line_node_t));
#endif
                     /* Link in at top of list */
                     if (next == line_anchor)
                     {
                        line_anchor = p;
                        p->line_ptr = next;
                     }
                     /* Link into middle */
                     else
                     {
                        back->line_ptr = p;
                        p->line_ptr = next;
                     }
                  }
 
                  /* Replace old with new */
                  else
                  {
                     pcp = p->line_txt;
                     while (*pcp && *pcp != ' ') pcp++;
                     if (*pcp != EOL)
                     {
                        strcpy (next->line_txt, p->line_txt);
                     }
                     /* Null input, delete */
                     else
                     {
                        back->line_ptr = next->line_ptr;
                        freeline (next);
                     }
                     freeline (p);
                  }
 
               }
            }
            else
            {
               error (BAD_LINE, 0);
               freeline (p);
            }
         }
         else
         {
            error (BAD_LINE, 0);
            freeline (p);
         }
      }
   }
 
} /* insertline */
 
/************************************************************************
*
* LIBRARYCMD - Provide library services
*
* Procedure LIBRARY processes the librarian functions. Syntax is:
*   L(IBRARY) C(ALL) <PATHNAME>   reads program from file
*   L(IBRARY) L(IST) [<PATHNAME>] list programs
*   L(IBRARY) P(RINT) <PATHNAME>  sends type/write output to file
*   L(IBRARY) S(AVE) <PATHNAME>   saves program in file
*   L(IBRARY) W(ORK) <PATHNAME>   change working directory
*
************************************************************************/

static void
librarycmd (void)
{
#if defined(BSDDIR) || defined(SYSVDIR)
   DIR *dir;
#endif
   line_node_t *spc, *p, tp;
   char ch, *bp, *mode;
   int done, k;
   char fname[PATH_LEN+1];
 
   /* Position to function */
   nextfield();
 
   /* Get function (C,S,P) */
   ch = *pcp;
   nextfield();
 
   /* Scan off pathname */
   bp = tbuf;
   while (*pcp && *pcp != EOL && *pcp != ';' && *pcp != ' ') *bp++ = *pcp++;
   *bp = EOL;
   fname[0] = EOL;

#ifdef DIGNUS
   strcpy (fname, "//DSN:");
#endif
 
   switch (upcase (ch))
   {
 
      case 'C' : /* Call (load) program */
         if (tbuf[0] != EOL)
         {
#if defined(DX10)
	    if (wd[0])
	       sprintf (fname, "%s.", wd);
#elif defined(BSDDIR) || defined(SYSVDIR)
	    if (tbuf[0] != '/')
	       sprintf (fname, "%s/", wd);
#endif
	    strcat (fname, tbuf);
         }
         else
         {
	    fprintf (stderr, "Filename is required");
	    error (FILE_ERROR, 0);
            return;
         }
#if defined(OS390)
         for (bp = fname; *bp; bp++)
	    *bp = upcase (*bp);
#elif !defined(DX10)
         /* Append extension */
 
         if ((bp = (char *)strrchr (tbuf, '.')) == NULL)
         {
            strcat (fname, FOCAL_EXT);
         }
#endif
 
         if ((progfile = fopen (fname, "r")) != NULL)
         {
            spc = pc;
            mode = pcp;
            done = FALSE;
            while (!done)
            {

               if (fgets (tbuf, sizeof (tbuf), progfile) != NULL)
               {
		  if ((bp = strchr (tbuf, '\n')) != NULL)
		     *bp = EOL;
                  k = strlen (tbuf);
#ifdef DEBUG_FILE
                  printf ("tbuf(%d) = '%s'\n", k, tbuf);
#endif
		  memset ((void *)&tp, 0, sizeof(line_node_t));
                  pc = &tp;
                  pcp = pc->line_txt;
                  strcpy (pcp, tbuf);
                  if (isdigit (*pcp))
                     insertline();
               }
               else
                  done = TRUE;
            }
            pcp = mode;
            pc = spc;
            fclose (progfile);
         }
         else
         {
            fprintf (stderr, "Can't open for read : %s", fname);
#if defined(STRERROR)
            fprintf (stderr, " : %s", strerror (errno));
#else
            perror (" ");
#endif
            error (FILE_ERROR, errno);
         }
         break;
 
      case 'L' : /* L(ist) files */
#if (defined(BSDDIR) || defined(SYSVDIR)) && !defined(MINFOCAL)
         if (tbuf[0] == EOL)
            strcpy (tbuf, wd);
         if ((dir = opendir (tbuf)) != NULL)
         {
#if defined(BSDDIR)
            struct direct *myent;
#endif
#if defined(SYSVDIR)
            struct dirent *myent;
#endif
            while ((myent = readdir (dir)) != NULL)
            {
#ifdef DX10
	       if ((myent->d_flags & 0xC000) == 0)
		  printf (" %s\n", myent->d_name);
#else
               char *bp;

               if ((bp = strrchr (myent->d_name, '.')) != NULL)
               {
                  if (!strcmp (bp, FOCAL_EXT))
		  {
		     struct stat stbuf;

		     sprintf (fname, "%s/%s", tbuf, myent->d_name);
		     if (stat (fname, &stbuf) == 0)
		     {
			printf ("%6ld %s\n", stbuf.st_size, myent->d_name);
		     }
		     else
		     {
			fprintf (stderr, "Can't stat file : %s", myent->d_name);
#if defined(STRERROR)
			fprintf (stderr, " : %s", strerror (errno));
#else
			perror (" ");
#endif
			error (FILE_ERROR, errno);
		     }
		  }
               }
#endif
            }
            closedir (dir);
         }
         else
         {
            fprintf (stderr, "Can't open directory : %s", tbuf);
#if defined(STRERROR)
	    fprintf (stderr, " : %s", strerror (errno));
#else
	    perror (" ");
#endif
            error (FILE_ERROR, errno);
         }
#else
         fprintf (stderr, "List not supported for this OS");
	 error (FILE_ERROR, 0);
#endif
         break;

      case 'P' : /* Print to new file */
         if (pndx > PBEG)
         {
            fputs ("\n", output);
            pndx = PBEG;
         }
 
         if (output != stdout)
            fclose (output);
 
         if (!strcmp (tbuf, "TTY") ||
	     !strcmp (tbuf, "tty"))
         {
            output = stdout;
         }
 
         else
         {
	    if (tbuf[0] != EOL)
	    {
#if defined(DX10)
	       if (wd[0])
		  sprintf (fname, "%s.", wd);
#elif defined(BSDDIR) || defined(SYSVDIR)
	       if (tbuf[0] != '/')
		  sprintf (fname, "%s/", wd);
#endif
	       strcat (fname, tbuf);
	    }
	    else
	    {
	       fprintf (stderr, "Filename is required");
	       error (FILE_ERROR, 0);
	       return;
	    }
#if defined(OS390)
	    for (bp = fname; *bp; bp++)
	       *bp = upcase(*bp);
#elif !defined(DX10)
	    /* Append extension */
    
	    if ((bp = (char *)strrchr (tbuf, '.')) == NULL)
	    {
	       strcat (fname, FOCAL_LST);
	    }
#endif
    
	    if ((output = fopen (fname, "w")) == NULL)
	    {
	       output = stdout;
	       fprintf (stderr, "Can't open for write : %s", fname);
#if defined(STRERROR)
	       fprintf (stderr, " : %s", strerror (errno));
#else
	       perror (" ");
#endif
	       error (FILE_ERROR, errno);
	    }
         }
         break;
 
      case 'S' : /* Save program */
         if (tbuf[0] != EOL)
         {
#if defined(DX10)
	    if (wd[0])
	       sprintf (fname, "%s.", wd);
#elif defined(BSDDIR) || defined(SYSVDIR)
	    if (tbuf[0] != '/')
	       sprintf (fname, "%s/", wd);
#endif
	    strcat (fname, tbuf);
         }
         else
         {
	    fprintf (stderr, "Filename is required");
	    error (FILE_ERROR, 0);
            return;
         }
#if defined(OS390)
         for (bp = fname; *bp; bp++)
	    *bp = upcase(*bp);
#elif !defined(DX10)
         /* Append extension */
 
         if ((bp = (char *)strrchr (tbuf, '.')) == NULL)
         {
            strcat (fname, FOCAL_EXT);
         }
#endif
 
         if ((progfile = fopen (fname, "w")) != NULL)
         {
            p = line_anchor;
            while (p != NULL)
            {
               fprintf (progfile, "%s.%s %s\n",
			p->grp_num, p->stp_num, p->line_txt);
               p = p->line_ptr;
            }
            fclose (progfile);
         }
         else
         {
            fprintf (stderr, "Can't open for write : %s", fname);
#if defined(STRERROR)
            fprintf (stderr, " : %s", strerror (errno));
#else
	    perror (" ");
#endif
            error (FILE_ERROR, errno);
         }
         break;
 
      case 'W': /* Change Working directory */
#if (defined(BSDDIR) || defined(SYSVDIR)) && !defined(MINFOCAL)
	 if (tbuf[0] != EOL)
	 {
#if defined(DX10)
	    strcpy (wd, tbuf);
#else
	    if ((dir = opendir (tbuf)) != NULL)
	    {
	       closedir (dir);
	       strcpy (wd, tbuf);
	    }
	    else
	    {
	       fprintf (stderr, "Can't open directory : %s", tbuf);
#if defined(STRERROR)
	       fprintf (stderr, " : %s", strerror (errno));
#else
	       perror (" ");
#endif
	       error (FILE_ERROR, errno);
	    }
#endif
	 }
	 else
	 {
	    fprintf (stderr, "Directory is required");
	    error (FILE_ERROR, 0);
	 }
#else
         fprintf (stderr, "Working directory not supported for this OS");
	 error (FILE_ERROR, 0);
#endif
	 break;

      default:
         error (BAD_CMD, LIBRARY_CMD);
 
   }
 
} /* librarycmd */
 
/************************************************************************
*
* MODIFYCMD - Modify source line
*
* Procedure MODIFY fixes up source lines and has the following
* syntax:
*       M(ODIFY) GG.SS /OLD/NEW/
*
************************************************************************/

static void
modifycmd (void)
{
   line_node_t *l;
   int start_pos, match_pos;
   int found, done;
   int i, j, k, disp;
   int new_len, old_len;
   char delim;
 
   nextfield();
   l = findline();
   if (l != NULL)
   {
      nextfield();
      if (*pcp != EOL)
      {
         delim = *pcp++;
         start_pos = (int)(pcp - pc->line_txt);
         j = 0;
         done = FALSE;
         while (!done)
         {
            k = start_pos;
            while (l->line_txt[j] != pc->line_txt[k] &&
                   l->line_txt[j] != EOL) j++;
            match_pos = j;
            if (l->line_txt[j] != EOL)
            {
               found = FALSE;
               do
               {
                  if (l->line_txt[j] == pc->line_txt[k])
                  {
                     j++;
                     k++;
                     if (pc->line_txt[k] == delim)
                        found = TRUE;
                  }
                  else
                     break;
               } while (!found && l->line_txt[j] != EOL);
               if (found)
               {
                  k++;
                  i = k;
                  pcp = &pc->line_txt[k];
                  while (*pcp && *pcp != delim && *pcp != EOL)
                  {
                     k++;
                     pcp++;
                  }
                  if (*pcp == delim) pcp++;
                  new_len = k - i;
                  old_len = j - match_pos;
                  /* Contract line */
                  if (old_len > new_len)
                  {
                     disp = old_len - new_len;
                     for (k = j; l->line_txt[k] != EOL; k++)
                        l->line_txt[k-disp] = l->line_txt[k];
                     l->line_txt[k-disp] = EOL;
                  }
                  /* Expand line */
                  else if (old_len < new_len)
                  {
                     disp = new_len - old_len;
                     for (k = strlen (l->line_txt)+disp; k > j; k--)
                        l->line_txt[k] = l->line_txt[k-disp];
                  }
                  /* Copy in new text */
                  for (j = 0; j < new_len; j++)
                     l->line_txt[match_pos++] = pc->line_txt[i++];
                  done = TRUE;
               }
               else
                  j = match_pos + 1;
            }
            else
            {
               done = TRUE;
               error (MODIFY_ERROR, 0);
            }
         } /* while */
      }
   }
   else
      error (BAD_LINE, 0);
 
} /* modifycmd */
 
/************************************************************************
*
* NEXTFIELD - Skip to next field
*
* Procedure NEXT_FIELD moves the pointer forward in the buffer
* to the next non_blank field, end of command (;) or end of line.
*
************************************************************************/

static void
nextfield (void)
{
 
   while (*pcp && *pcp != EOL && *pcp != ' ' && *pcp != ';') pcp++;
   while (*pcp == ' ') pcp++;
 
} /* nextfield */
 
/************************************************************************
*
* PCPOP - Pops the PC context
*
* This routine pops the context from the PC stack.
*
************************************************************************/

static void
pcpop (void)
{
   pc_stk_t *p;
 
   p = pc_top;
   pc_top = p->pc_ptr;
#ifdef SMALL_MEMORY
   free (p);
#else
   p->pc_ptr = free_pc;
   free_pc = p;
#endif
 
} /* pcpop */
 
/************************************************************************
*
* PCPUSH - Push PC context
*
* This routine pushes the program context on the PC stack.
*
************************************************************************/

static void
pcpush (void)
{
   pc_stk_t *p;
 
#ifdef SMALL_MEMORY
   if ((p = (pc_stk_t *)malloc (sizeof (pc_stk_t))) == NULL)
   {
      error (MEM_OVERFLOW, STACK_OVERFLOW);
   }
#else
   if (free_pc == NULL)
   {
      if ((p = (pc_stk_t *)malloc (sizeof (pc_stk_t))) == NULL)
      {
         error (MEM_OVERFLOW, STACK_OVERFLOW);
      }
   }
   else
   {
      p = free_pc;
      free_pc = p->pc_ptr;
   }
#endif
 
   if (p != NULL)
   {
      memset ((void *)p, 0, sizeof (pc_stk_t));

      p->pc_index = pcp;
      p->old_pc = pc;
      p->pc_ptr = pc_top;
      pc_top = p;
   }
 
} /* pcpush */
 
/************************************************************************
*
* QUIT - Quit command
*
* This routine processes the quit command.
*
************************************************************************/

static void
quitcmd (void)
{
   do_stk_t *p;
   for_stk_t *q;
 
   nextfield();
   if (!(run_mode || do_mode))
      quit_flag = TRUE;
   run_mode = FALSE;
   do_mode = FALSE;
 
   while (pc_top != NULL)
      pcpop();
 
   while (do_top != NULL)
   {
      p = do_top;
      do_top = p->do_ptr;
      freedo (p);
   }
 
   while (for_top != NULL)
   {
      q = for_top;
      for_top = q->for_ptr;
      freefor (q);
   }
 
} /* quitcmd */
 
/************************************************************************
*
* RETURNCMD - Return command
*
* This routine processes the return command.
*
************************************************************************/

static void
returncmd (void)
{
   do_stk_t *p;
   for_stk_t *q;
 
   nextfield();
   if (do_top != NULL)
   {
      while (pc_top->pc_flags != DO_FLG)
      {
         pcpop();
         q = for_top;
         for_top = q->for_ptr;
         freefor (q);
      }
      pc = pc_top->old_pc;
      pcp = pc_top->pc_index;
      if (trace_flag)
      {
	 fprintf (stdout, "\nR%s.%s ", pc->grp_num, pc->stp_num);
      }
      pcpop();
      p = do_top;
      do_top = p->do_ptr;
      freedo (p);
      if (do_top == NULL)
         do_mode = FALSE;
   }
 
} /* returncmd */
 
/************************************************************************
*
* SETCMD - Set command
*
* This routine processes the set command. Syntax:
*   S(ET) <VAR> = <EXPR>
*
************************************************************************/

static void
setcmd (void)
{
   tokval val;
   int  ndx;
   char sym[TWOCHAR+1];
 
   nextfield();
 
   /* Error if function  */
   if (upcase (*pcp) == 'F')
   {
      error (BAD_FUNC, 0);
      return;
   }
 
   getsym (sym);
   if (*pcp == ' ')
      nextfield();
 
   ndx = 0;
   if (*pcp == '(' || *pcp == '<' || *pcp == '[' || *pcp == '{')
   {
      ndx = expression();
   }
 
   if (!err_flag)
   {
      if (*pcp == ' ')
         nextfield();
      if (*pcp == '=')
      {
         pcp++;
         val = expression();
         if (!err_flag)
            symboltable (sym, &val, ndx, FALSE);
      }
      else
         error (BAD_EXPR, FOR_EXPR);
   }
 
} /* setcmd */
 
/************************************************************************
*
* SYMBOLTABLE - Process symbol table
*
* This routine stores and/or retrieves data from the symbol table.
* if the symbol is not found it is created with an initial value of
* zero.
*
************************************************************************/

void
symboltable (char *sym, tokval *val, int ndx, int flg)
{
   sym_node_t *p, *next;
   int j, found;
   char lsym[TWOCHAR+1];
 
#ifdef DEBUG_SYMTAB
   printf ("symboltable: sym = '%s', ndx = %d, flg = %s\n",
	   sym, ndx, flg ? "TRUE" : "FALSE");
#endif
   strcpy (lsym, "  ");
   for (j = 0; j < strlen (sym); j++)
      if (j < TWOCHAR)
         lsym[j] = sym[j];
 
#ifdef SMALL_MEMORY
   if ((p = (sym_node_t *)malloc (sizeof (sym_node_t))) == NULL)
   {
      error (MEM_OVERFLOW, SYMBOL_OVERFLOW);
   }
#else
   if (free_sym == NULL)
   {
      if ((p = (sym_node_t *)malloc (sizeof (sym_node_t))) == NULL)
      {
         error (MEM_OVERFLOW, SYMBOL_OVERFLOW);
      }
   }
   else
   {
      p = free_sym;
      free_sym = p->sym_ptr;
   }
#endif
 
   if (p != NULL)
   {
      memset ((void *)p, 0, sizeof (sym_node_t));

      /* Initialize new node */
      p->sym_ptr = NULL;
      strcpy (p->symbol, lsym);
      p->sym_index = ndx;
      p->sym_value = *val;
      next = sym_anchor;
 
      /* If list empty add at head */
      if (sym_anchor == NULL)
         sym_anchor = p;
 
      else
      {
 
         found = FALSE;
         while (next != NULL && !found)
            if (!strcmp (next->symbol, lsym) &&
		(next->sym_index == ndx))
               found = TRUE;
            else
               next = next->sym_ptr;
 
         /* Symbol not found */
         if (next == NULL)
         {
            /* Link new sym at head */
            p->sym_ptr = sym_anchor;
            sym_anchor = p;
         }
 
         /* Symbol is found */
         else
         {
            freesym (p);
            /* If flag is true */
            if (flg)
            {
               /* Return current value */
               *val = next->sym_value;
            }
            else
            {
               /* Set new value */
               next->sym_value = *val;
            }
         }
      }
   }
 
} /* symboltable */
 
/************************************************************************
*
* TYPECMD - Type command
*
* this procedure processes the type command. The recognized forms are
* as follows:
*       T(YPE) <VAR>        type a variable
*       T(YPE) <EXPR>       type an expression
*       T(YPE) "TEXT"       type a text string
*
************************************************************************/

static void
typecmd (void)
{
   sym_node_t *p;
   tokval val;
   int  k, j;
   char delim;
   char row[TWOCHAR+1], col[TWOCHAR+1];
 
   nextfield();
   do
   {
 
      /* Buffer overflow print line */
      if (pndx >= PLEN)
      {
         fputc ('\n', output);
         pndx = PBEG;
      }
 
      switch (*pcp)
      {
 
         case '"' : /* Start of text string */
         case '\'' :
	    if (trace_flag)
	    {
	       fputc (*pcp, output);
	    }
            delim = *pcp++;
            while (*pcp && *pcp != EOL && *pcp != delim)
            {
               fputc (*pcp++, output);
               pndx++;
            }
            if (*pcp == EOL)
               error (BAD_STRING, 0);
            else
               pcp++;
	    if (trace_flag)
	    {
	       fputc (delim, output);
	    }
            break;
 
         case '!' : /* Carriage return/line feed */
            pcp++;
            fputc ('\n', output);
            pndx = PBEG;
            break;
 
         case '#' : /* Carriage return */
            pcp++;
            fputc ('\r', output);
            pndx = PBEG;
            break;
 
         case '&' : /* Top of form */
            pcp++;
            fputc ('\f', output);
            pndx = PBEG;
            break;
 
         case '$' : /* Print symbol table */
            pcp++;
            if (pndx > PBEG)
            {
               fputc ('\n', output);
               pndx = PBEG;
            }
 
            p = sym_anchor;
            while (p != NULL)
            {
#if defined(DX10)
               checkterm();
#endif
               if (user_stop)
               {
                  user_stop = FALSE;
                  return;
               }
               fprintf (output, "%s(%d) ", p->symbol, p->sym_index);
               fmtnum (p->sym_value);
               fputc ('\n', output);
               p = p->sym_ptr;
            }
            break;
 
         case '%' : /* Change numeric format */
            pcp++;
            width = 0;
            digits = 0;
            if (isdigit (*pcp))
            {
               getgrp (tbuf);
               width = Parser (tbuf);
               if (*pcp == '.')
               {
                  pcp++;
                  getgrp (tbuf);
                  digits = Parser (tbuf);
               }
            }
            break;
 
         case ':' : /* TAB */
            pcp++;
            k = expression() + PBEG;
            if (k > pndx)
               for (j = pndx; j < k; j++)
                  fputc (' ', output);
            pndx = k;
            break;
 
#if defined(ANSICRT)
         case '@' : /* Position on the screen */
            pcp++;
            strcpy (row, "01");
            strcpy (col, "01");
 
            if (upcase (*pcp) == 'E')
            {
               pcp++;
               clearscreen();
            }
 
            if (isdigit (*pcp))
            {
               getgrp (row);
               if (*pcp == '.')
               {
                  pcp++;
                  getgrp (col);
               }
               screenposition (row, col);
            }
 
            if (upcase (*pcp) == 'C')
            {
               pcp++;
               clearline();
            }
            break;
#endif
 
         case ' ' :
         case ',' :
            pcp++;
            break;
 
         default : /* print value of expression */
            val = expression();
            if (!err_flag)
               fmtnum (val);
 
      } /* of switch */
 
   } while (*pcp && *pcp != EOL && *pcp != ';' && !err_flag);
 
#if defined(OS390) || defined(OS2)
   fflush (output);
#endif
 
} /* typecmd */
 
/************************************************************************
*
* WRITECMD - Write lines command
*
* This procedure processes the write command. The recognized forms are
* as follows:
*      W(RITE) [A(LL)]      write out entire buffer
*      W(RITE) GRP          write out a group of lines
*      W(RITE) GRP.STP      write out a single line
*
************************************************************************/

static void
writecmd (void)
{
   line_node_t *p, *back;
   int found;
   char stp[TWOCHAR+1], grp[TWOCHAR+1];
 
   nextfield();
   if (pndx > PBEG)
   {
      fputc ('\n', output);
      pndx = PBEG;
   }
 
   /* List entire program */
   if (upcase (*pcp) == 'A' || *pcp == EOL || *pcp == ';')
   {
      nextfield();
      p = line_anchor;
      back = line_anchor;
 
      while (p != NULL)
      {
#if defined(DX10)
	 checkterm();
#endif
         if (user_stop)
         {
            user_stop = FALSE;
            return;
         }
#ifdef DEBUG_FILE
	 printf ("p = %p\n", p);
	 HEXDUMP (output, (char *)p, sizeof (line_node_t));
#endif
         fprintf (output, "%s.%s %s\n", p->grp_num, p->stp_num, p->line_txt);
         back = p;
         p = p->line_ptr;
         if (p != NULL)
            if (strcmp (back->grp_num, p->grp_num))
               fputc ('\n', output);
      }
 
   }
 
   /* List a group of lines */
   else if (isdigit (*pcp))
   {
      getgrp (grp);
      p = line_anchor;
      found = FALSE;
 
      while (p != NULL && !found)
         if (!strcmp (p->grp_num, grp))
            found = TRUE;
         else
            p = p->line_ptr;
 
      if (p != NULL)
      {
         /* list one line */
         if (*pcp == '.')
         {
            pcp++;
            getstp (stp);
            if (!err_flag)
            {
               found = FALSE;
               while (p != NULL && !found)
                  if (!strcmp (p->grp_num, grp) &&
                      !strcmp (p->stp_num, stp))
                     found = TRUE;
                  else
                     p = p->line_ptr;
            }
            if (p != NULL)
               fprintf (output, "%s.%s %s\n",
			p->grp_num, p->stp_num, p->line_txt);
         }
 
         /* Its a group */
         else
         {
            found = FALSE;
            while (p != NULL && !found)
               if (!strcmp (p->grp_num, grp))
               {
                  fprintf (output, "%s.%s %s\n",
			   p->grp_num, p->stp_num, p->line_txt);
#if defined(DX10)
		  checkterm();
#endif
                  if (user_stop)
                  {
                     user_stop = FALSE;
                     return;
                  }
                  p = p->line_ptr;
               }
               else
                  found = TRUE;
         }
      }
   }
 
   else ; /* Invalid write request */
 
} /* writecmd */
 
#if !defined(DX10)
/***********************************************************************
* fpeint - floating point exception
***********************************************************************/
 
void
fpeint (int sig)
{
   fpe_stop = TRUE;
   signal (SIGFPE, fpeint);
}
 
/***********************************************************************
* userint - user keyboard interrupt
***********************************************************************/
 
void
userint (int sig)
{
   user_stop = TRUE;
   signal (SIGINT, userint);
}
#endif
 
/**********************************************************************
*
* Main driver
*
**********************************************************************/

int
main (int argc, char *argv[])
{
 
   /* Index into print buffer */
   output = stdout;
   input = stdin;
   pndx = PBEG;
 
   /* Set default width */
   width = 10;
 
   /* Set default significance */
   digits = 4;
 
   /* Seed random number generator */
   time (&seed);
   srand (seed);
 
   /* Set list pointers to NULL */
   line_anchor = NULL;
   sym_anchor = NULL;
   for_top = NULL;
   pc_top = NULL;
   do_top = NULL;
#ifndef SMALL_MEMORY
   free_line = NULL;
   free_sym = NULL;
   free_for = NULL;
   free_pc = NULL;
   free_do = NULL;
#endif
 
   /* Set initial flags */
   quit_flag = FALSE;
   user_stop = FALSE;
   fpe_stop = FALSE;
   run_mode = FALSE;
   do_mode = FALSE;
 
   /* Allocate keyboard buffer */
   buffer = (line_node_t *)malloc (sizeof (line_node_t));
   memset ((void *)buffer, 0, sizeof (line_node_t));
 
#if !defined(DX10)
   /* Initialize keyboard event */
   signal (SIGINT, userint);

   /* Initialize Floating point exception */
   signal (SIGFPE, fpeint);
#endif

   /* Set the working directory */
#if defined(DX10)
   wd[0] = '\0';
#else
   strcpy (wd, ".");
#endif
 
   /* If an arg, then execute it */
   if (argc == 2)
   {
      pc = buffer;
      err_flag = FALSE;
      trace_flag = FALSE;
      pndx = PBEG;
      pcp = pc->line_txt;
 
      sprintf (pc->line_txt, "L C %s;G%c", argv[1], EOL);
      execline();
      return (NORMAL);
   }

   /* Print the banner */
   fprintf (stdout, "FOCAL-%s execution begins\n", VERSION);

   /* Process until quit command or EOF */
   while (!quit_flag)
   {
      pc = buffer;
      err_flag = FALSE;
      trace_flag = FALSE;
      pndx = PBEG;
 
      /* Prompt user for input */
      fputc ('*', stdout);
#if defined(OS390) || defined(OS2)
      fputc ('\n', stdout);
#endif
#if defined(OS390) || defined(OS2) || defined(DX10)
      fflush (stdout);
#endif
 
      if ((pcp = fgets (pc->line_txt, LINE_LEN, stdin)) != NULL)
      {
         *((char *)(strchr (pcp, '\n'))) = EOL;
#ifdef DEBUG_INPUT
	 printf ("input = '%s'\n", pcp);
#endif
 
         /* Store line */
         if (isdigit (*pcp))
            insertline();
 
         /* Process command */
         else
            execline();
      }
      else
         quit_flag = TRUE;
   }

   return (NORMAL);
 
} /* focal */

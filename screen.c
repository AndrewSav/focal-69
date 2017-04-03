#if defined(OS390)
#pragma nomargins
#pragma nosequence
#endif
 
/***********************************************************************
*
* Screen control routines, Uses ANSI control sequences.
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2, or (at your option) any
* later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
***********************************************************************/

#include <stdio.h>

#if defined(OPENEDITION) || defined(OS390)
#define ESCAPE  0x27
#else
#define ESCAPE  0x1B
#endif
 
/***********************************************************************
* clearscreen - clear the screen
***********************************************************************/
 
void
clearscreen (void)
{
   fprintf (stdout, "%c[2J", ESCAPE);
}
 
/***********************************************************************
* clearline - clear the line
***********************************************************************/
 
void
clearline (void)
{
   fprintf (stdout, "%c[2K", ESCAPE);
}
 
/***********************************************************************
* screen position - position on the screen
***********************************************************************/
 
void
screenposition (char *row, char *col)
{
   fprintf (stdout, "%c[%s;%sH", ESCAPE, row, col);
}

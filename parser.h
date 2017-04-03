/***********************************************************************
* File = parser.h
***********************************************************************/

#define SYMLEN  10

typedef char            int8;
typedef short           int16;
typedef long            int32;
typedef unsigned char   byte;
typedef unsigned short  word;
typedef unsigned long   dword;

typedef byte            tchar;  /* 0..255 */
typedef byte            pstate; /* 0..255 */
typedef int8            toktyp; /* 0..0x7F */
typedef double          tokval;

#if defined(OS390)
#define Parser PARSER
#endif
extern tokval Parser ( );


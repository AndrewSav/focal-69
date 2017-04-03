/***********************************************************************
* File = errors.h
***********************************************************************/

#define USER_STOP       0
#define BAD_CMD         1
#define   LIBRARY_CMD      1
#define   COMMAND          2
#define BAD_LINE        2
#define   GO_TARGET        1
#define   DO_TARGET        2
#define BAD_VAR         3
#define BAD_EXPR        4
#define   FOR_EXPR         1
#define   IF_EXPR          2
#define   TAB_EXPR         3
#define BAD_FUNC        5
#define BAD_STRING      6
#define BAD_NUM         7
#define MEM_OVERFLOW    8
#define    LINE_OVERFLOW   1
#define    SYMBOL_OVERFLOW 2
#define    STACK_OVERFLOW  3
#define PARSE_ERROR     9
#define SCAN_ERROR      10
#define    SCAN_SIGN       9
#define    SCAN_FRAC       7
#define    SCAN_EXPON      8
#define INTERP_ERROR    11
#define    ZERO_DIVIDE     1
#define    NEG_SQRT        2
#define    NEG_LOG         3
#define MODIFY_ERROR    12
#define FILE_ERROR      13
#define FPE_STOP        14
#define UNDEF_FUNC      15


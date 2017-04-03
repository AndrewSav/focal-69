/*****************************************************************
* Parser tables, Generated at Mon Dec  7 14:33:13 2009
*****************************************************************/
/*
** Parser action macros
*/
#define SHIFT(co,ar)  (ar<<8|co)
#define REDUCE(co)    (co<<8|255)
#define ERROR         -1
#define GOTO(c,n)     (n<<8|(c&255))

/*
** Parser token type equates
*/
#define EOS     3
#define NUM     15
#define VAR     17
#define FNAME   19

/*
** SLR(1) parser action tables
*/
static short int P1[] = {
       SHIFT('-',4),
       SHIFT('(',7),
       SHIFT(NUM,9),
       SHIFT(VAR,11),
       SHIFT(FNAME,13),
       ERROR
};
static short int P2[] = {
       SHIFT(EOS,14),
       SHIFT('+',16),
       SHIFT('-',15),
       ERROR
};
static short int P3[] = {
       SHIFT('*',17),
       SHIFT('/',18),
       REDUCE(2)
};
static short int P4[] = {
       SHIFT('(',7),
       SHIFT(NUM,9),
       SHIFT(VAR,11),
       SHIFT(FNAME,13),
       ERROR
};
static short int P5[] = {
       SHIFT('^',20),
       REDUCE(6)
};
static short int P6[] = {
       REDUCE(9)
};
static short int P7[] = {
       SHIFT('-',4),
       SHIFT('(',7),
       SHIFT(NUM,9),
       SHIFT(VAR,11),
       SHIFT(FNAME,13),
       ERROR
};
static short int P8[] = {
       REDUCE(12)
};
static short int P9[] = {
       REDUCE(13)
};
static short int P10[] = {
       REDUCE(14)
};
static short int P11[] = {
       SHIFT('(',22),
       REDUCE(15)
};
static short int P12[] = {
       REDUCE(16)
};
static short int P13[] = {
       SHIFT('(',23),
       ERROR
};
static short int P14[] = {
       REDUCE(1)
};
static short int P15[] = {
       SHIFT('(',7),
       SHIFT(NUM,9),
       SHIFT(VAR,11),
       SHIFT(FNAME,13),
       ERROR
};
static short int P16[] = {
       SHIFT('(',7),
       SHIFT(NUM,9),
       SHIFT(VAR,11),
       SHIFT(FNAME,13),
       ERROR
};
static short int P17[] = {
       SHIFT('(',7),
       SHIFT(NUM,9),
       SHIFT(VAR,11),
       SHIFT(FNAME,13),
       ERROR
};
static short int P18[] = {
       SHIFT('(',7),
       SHIFT(NUM,9),
       SHIFT(VAR,11),
       SHIFT(FNAME,13),
       ERROR
};
static short int P19[] = {
       SHIFT('*',17),
       SHIFT('/',18),
       REDUCE(3)
};
static short int P20[] = {
       SHIFT('(',7),
       SHIFT(NUM,9),
       SHIFT(VAR,11),
       SHIFT(FNAME,13),
       ERROR
};
static short int P21[] = {
       SHIFT(')',29),
       SHIFT('+',16),
       SHIFT('-',15),
       ERROR
};
static short int P22[] = {
       SHIFT('-',4),
       SHIFT('(',7),
       SHIFT(NUM,9),
       SHIFT(VAR,11),
       SHIFT(FNAME,13),
       ERROR
};
static short int P23[] = {
       SHIFT('-',4),
       SHIFT('(',7),
       SHIFT(NUM,9),
       SHIFT(VAR,11),
       SHIFT(FNAME,13),
       ERROR
};
static short int P24[] = {
       SHIFT('*',17),
       SHIFT('/',18),
       REDUCE(5)
};
static short int P25[] = {
       SHIFT('*',17),
       SHIFT('/',18),
       REDUCE(4)
};
static short int P26[] = {
       SHIFT('^',20),
       REDUCE(7)
};
static short int P27[] = {
       SHIFT('^',20),
       REDUCE(8)
};
static short int P28[] = {
       REDUCE(10)
};
static short int P29[] = {
       REDUCE(11)
};
static short int P30[] = {
       SHIFT(')',32),
       SHIFT('+',16),
       SHIFT('-',15),
       ERROR
};
static short int P31[] = {
       SHIFT(')',33),
       SHIFT('+',16),
       SHIFT('-',15),
       ERROR
};
static short int P32[] = {
       REDUCE(17)
};
static short int P33[] = {
       REDUCE(18)
};

static short int const *parsetable[] = {
       P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, 
       P12, P13, P14, P15, P16, P17, P18, P19, P20, P21, 
       P22, P23, P24, P25, P26, P27, P28, P29, P30, P31, 
       P32, P33
};

/*
** SLR(1) parser goto tables
*/
static short int GOAL[] = {
       GOTO(-1,0)
};
static short int EXPR[] = {
       GOTO(1,2),
       GOTO(7,21),
       GOTO(22,30),
       GOTO(-1,31)
};
static short int TERM[] = {
       GOTO(4,19),
       GOTO(15,24),
       GOTO(16,25),
       GOTO(-1,3)
};
static short int FACT[] = {
       GOTO(17,26),
       GOTO(18,27),
       GOTO(-1,5)
};
static short int PRIM[] = {
       GOTO(20,28),
       GOTO(-1,6)
};
static short int VARBL[] = {
       GOTO(-1,8)
};
static short int SUBV[] = {
       GOTO(-1,12)
};
static short int FUNC[] = {
       GOTO(-1,10)
};

static struct {
   short int *go;
   int  handle;
} const gototable[] = {
   /* G1 */ GOAL,2,
   /* G2 */ EXPR,1,
   /* G3 */ EXPR,2,
   /* G4 */ EXPR,3,
   /* G5 */ EXPR,3,
   /* G6 */ TERM,1,
   /* G7 */ TERM,3,
   /* G8 */ TERM,3,
   /* G9 */ FACT,1,
   /* G10 */ FACT,3,
   /* G11 */ PRIM,3,
   /* G12 */ PRIM,1,
   /* G13 */ PRIM,1,
   /* G14 */ PRIM,1,
   /* G15 */ VARBL,1,
   /* G16 */ VARBL,1,
   /* G17 */ SUBV,4,
   /* G18 */ FUNC,4
};

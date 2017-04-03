FOCAL (EXECUTE FOCAL INTERPRETER) = 0,
************************************************************************
* PURPOSE: EXECUTE FOCAL INTERPRETER.
* INPUTS:
*   PROGRAM:     FILE CONTAINING A FOCAL PROGRAM.
*
* FOCAL REQUIRES A /12 CPU AS IT USES HARDWARE FLOATING POINT.
*
************************************************************************
      PROGRAM = *ACNM(@$CCP)
*
* BID THE TASK
*
.IF "@$$12", EQ, "N"
   MSG T="NOT A /12 CPU"
   .EXIT
.ENDIF
.SYN $CCP = "&PROGRAM"
.IF "&PROGRAM", NE, ""
   .BID TASK = FOCAL, LUNO=010, PARMS=(2,8,"FOCAL",@&PROGRAM)
.ELSE
   .BID TASK = FOCAL, LUNO=010, PARMS=(2,8,"FOCAL")
.ENDIF

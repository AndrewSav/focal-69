FOCAL (EXECUTE FOCAL INTERPRETER) = 0,
************************************************************************
* PURPOSE: EXECUTE FOCAL INTERPRETER.
* INPUTS:
*   NONE - Prompts for arguments.
*
* FOCAL REQUIRES A /12 CPU AS IT USES HARDWARE FLOATING POINT.
*
************************************************************************
*
* BID THE TASK
*
.IF "@$$12", EQ, "N"
   MSG T="NOT A /12 CPU"
   .EXIT
.ENDIF
.OVLY OVLY=>1B,LUNO=0,PARMS=(18,".S$SDS$", "FOCAL", "ME",
  2,12,Y,N,$$RI)

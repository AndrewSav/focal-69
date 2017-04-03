case 1:
   value = Stkval(2); 
   break;
case 3:
   value = - Stkval(1); 
   break;
case 4:
   value = Stkval(3) + Stkval(1); 
   break;
case 5:
   value = Stkval(3) - Stkval(1); 
   break;
case 7:
   value = Stkval(3) * Stkval(1); 
   break;
case 8:
   if (Stkval(1) == 0.0)
   Parse_Error (INTERP_ERROR, ZERO_DIVIDE);
   else
   value = Stkval(3) / Stkval(1); 
   break;
case 10:
   if (Stkval(3) == 0.0)
   value = 0.0;
   else
   value = pow(Stkval(3),Stkval(1)); 
   break;
case 11:
   value = Stkval(2); 
   break;
case 15:
   symboltable (CStkval(1), &value, 0, TRUE); 
   break;
case 17:
   k = Stkval(2);
   symboltable (CStkval(4), &value, k, TRUE); 
   break;
case 18:
   k = Stkval(4); switch (k) {
   case 1: /* fsqt */
   if (Stkval(2) < 0.0)
   Parse_Error (INTERP_ERROR, NEG_SQRT);
   else
   value = sqrt(Stkval(2));
   break;
   case 2: /* fabs */
   value = fabs(Stkval(2));
   break;
   case 3: /* fsgn */
   if (Stkval(2) >= 0.0) value =  1.0;
   else                  value = -1.0;
   break;
   case 4: /* fitr */
   {int fitr; fitr = Stkval(2); value = fitr;}
   break;
   case 5: /* fran */
   value = rand();
   while (value > 1.0) value /= 10.0;
   break;
   case 6: /* fexp */
   value = exp(Stkval(2));
   break;
   case 7: /* fsin */
   value = sin(Stkval(2));
   break;
   case 8: /* fcos */
   value = cos(Stkval(2));
   break;
   case 9: /* fatn */
   value = atan(Stkval(2));
   break;
   case 10: /* flog */
   if (Stkval(2) <= 0.0)
   Parse_Error (INTERP_ERROR, NEG_LOG);
   else
   value = log(Stkval(2));
   break;
   default:
   Parse_Error (UNDEF_FUNC, 0); } 
   break;

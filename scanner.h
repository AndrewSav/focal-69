
            case 1: /* Operator found */
               switch (lachar)
               {

                  case '[':
                  case '<':
                  case '{':
                     lachar = '(';
                  case '(':
                     token = lachar;
                     break;

                  case ']':
                  case '>':
                  case '}':
                     lachar = ')';
                  case ')':
                     token = lachar;
                     break;

                 default:
                     if (latran == 1)
                        token = EOS;
                     else
                        token = lachar;
               }
               break;

#define MAXFUNC 10
            case 2: /* Symbol found */
               {
                  int i;
                  static char *functable[MAXFUNC] = {
                        "FSQT", "FABS", "FSGN", "FITR", "FRAN",
                        "FEXP", "FSIN", "FCOS", "FATN", "FLOG" };

                  value = 0.0;
                  token = VAR;
                  for (i = 0; symbol[i] != '\0'; i++) 
                     symbol[i] = upcase(symbol[i]);
                  if (symbol[0] == 'F')
                  {
                     token = FNAME;
                     for (i = 0; i<MAXFUNC; i++)
                     {
                        if (strcmp (symbol, functable[i]) == 0)
                        {
                           value = i + 1;
                           break;
                        }
                     }
                  }
               }
               break;

            case 3: /* Number found */
               value = value * 10.0 + dignum;
               token = NUM;
               break;

            case 5: /* Exponent found */
               sexp = sexp * 10.0 + dignum;
               token = NUM;
               break;

            case 7: /* Fractional part */
               value = value + (sfrc * dignum);
               sfrc = sfrc / 10.0;
               token = NUM;
               break;

            case 8: /* Build exponential form */
               token = NUM;
               if (value != 0.0)
               {
                  if (sexp <= DBL_MAX_10_EXP)
                     value = value * pow (10.0, (sexp * expsgn));
                  else
                     Parse_Error (SCAN_ERROR, SCAN_EXPON);
               }
               break;

            case 9: /* Exponent sign */
               token = NUM;
               if (lachar == '-')
                  expsgn = -1.0;
               break;


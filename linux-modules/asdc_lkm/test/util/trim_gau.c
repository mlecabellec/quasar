/*******************************************************************/
/*   Suppression des caracteres blancs (et tabs) places a GAUCHE   */
/*   d'une chaine de caracteres                                    */
/*                                                                 */
/*                             Y. Guillemot, le 14 octobre 1989    */
/*                            derniere modif. le 3 fevrier 1989    */
/*******************************************************************/


#define tab   9
#define blanc 32

void YG_UTIL1_trim_gauche(ch)
char *ch;
{  char *p, *q;
   for(p=ch; (*p==blanc) || (*p==tab); p++);
   for(q=ch; *p!=0; q++, p++) *q = *p;
   *q=0;
}

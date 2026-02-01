/*******************************************************************/
/*   Suppression des caracteres blancs (et tabs) places a DROITE   */
/*   d'une chaine de caracteres                                    */
/*                                                                 */
/*                             Y. Guillemot, le 14 octobre 1989    */
/*                           derniere modif. le 20 octobre 1989    */
/*******************************************************************/


#define tab   9
#define blanc 32

void YG_UTIL1_trim_droite(ch)
char *ch;
{  char *p, *q;
   for(p=ch; *p!=0; p++);
   q=p-1;
   for(; ((*q==blanc) || (*q==tab)) && (p!=ch); p--, q--);
   *p=0;
}

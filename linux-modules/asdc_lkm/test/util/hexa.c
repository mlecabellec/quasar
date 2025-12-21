/*******************************************************************/
/*   Conversion d'un entier long en hexadecimal dans une chaine    */
/*   ASCII limitee a N caracteres (bits de poids fort tronques)    */
/*                                                                 */
/*   La fonction renvoie l'adresse de cette chaine                 */
/*                                                                 */
/*                             Y. Guillemot, le 20 octobre 1989    */
/*******************************************************************/


char *YG_UTIL1_hexa(val, N, chaine)
long int val;
int N;
char *chaine;
{ register int i;
  register int x;
  for (i=N-1; i>=0; i--)
    {  x = (val & 0x0000000F) + '0';
       if (x<='9') chaine[i]=x;
            else   chaine[i]=x+7;             /* 7 = 'A'-'9'-1; */
       val >>= 4;
    }
  chaine[N]=0;
  return(chaine);
}

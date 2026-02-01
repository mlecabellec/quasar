/*******************************************************************/
/*   Conversion d'un entier long en binaire dans une chaine ASCII  */
/*   limitee a N caracteres (bits de poids fort tronques)          */
/*                                                                 */
/*   La fonction renvoie l'adresse de cette chaine                 */
/*                                                                 */
/*                             Anonymized, le 20 octobre 1989    */
/*******************************************************************/


#define un   '1'
#define zero '0'

char *YG_UTIL1_bina(val, N, chaine)
long int val;
int N;
char *chaine;
{ register int i;
  for (i=N-1; i>=0; i--)
    {  if ((val & 0x00000001) == 1) chaine[i]=un;
                      else          chaine[i]=zero;
       val >>= 1;
    }
  chaine[N]=0;
  return(chaine);
}

/******************************************************************/
/*   Suppression des caracteres blancs (et tabs) places de part   */
/*   et d'autre d'une chaine de caracteres                        */
/*                                                                */
/*                            Anonymized, le 14 octobre 1989    */
/******************************************************************/


#define tab   9
#define blanc 32

void YG_UTIL1_trim(ch)
char *ch;
{  YG_UTIL1_trim_droite(ch);
   YG_UTIL1_trim_gauche(ch);
}

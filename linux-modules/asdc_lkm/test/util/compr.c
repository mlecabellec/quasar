/* CWRITE a le meme comportement que WRITE, mais effectue une compression     */
/* des donnees :                                                              */
/*    Un caractere isole est ecrit sans modifs                                */
/*    Deux a trois caracteres identiques consecutifs sont ecrits sans modifs  */
/*    Plus de trois caracteres identiques consecutifs (255 au maximum) sont   */
/*        ecrits de la facon suivante : " <esc> <n> <X> "                     */
/*            ou <esc> represente le caractere \033                           */
/*               <n> represente le caractere de code "nombre des caracteres"  */
/*               <X> represente le caractere a ecrire                         */
/*    Le caractere '\033' est toujours (meme isole) ecrit comme un groupe     */
/*            exemple :  "\033"     --> "\033\1\033"                          */
/*                       "\033\033" --> "\033\2\033" etc....                  */
/*                                                                            */
/* CREAD a le meme comportement que READ, mais recupere les donnees compres-  */
/* sees par CWRITE .                                                          */
/*                                                                            */
/*                                                                            */
/*                                                  Y. Guillemot - 1/8/1988   */

#include "noms_ext.h"

int cwrite(fd, buf, count)
int fd, count;
char *buf;
{char *i, *i1, *i2;            
 int nboct=0;
 i=i1=i2=buf;
 while (++i<buf+count) {
      if ((*i == *i1)&&((i2-i1)<254)) i2++;
          else   { nboct=nboct+cw_ecrire(fd,i1,i2);
                   i1=i2=i;
                 }
     }      
 nboct=nboct+cw_ecrire(fd,i1,i2); 
 return(nboct);
}

int cw_ecrire(fich,i1,i2)
int fich;
char *i1, *i2;       
{static char bb[4]={'\033','\0','\0','\0'};
 if (((i2-i1)<3) && (*i1!='\033')) return(write(fich,i1,i2-i1+1));
           else {bb[1]=(char) (i2-i1+1);
                 bb[2]=(char) *i1;  
                 if (write(fich,bb,3)==3) return(i2-i1+1);
                                   else   return(0);
                }
}
    





int cread(fd, buf, count)
int fd, count;
char *buf;
{char *i, bb[3];             
 int nboct=0;
 i=buf;
 while (count!=0) {  
          nboct=nboct+read(fd,i,1); 
          if (*i=='\033') {  
                  if (read(fd,bb,2)==2) {  
                       nboct--;
                       for (; bb[0]!=0; bb[0]--) {
                              *i++=bb[1];
                              nboct++;   
                              count--;}  
                       }
             
                 }  
                else { i++; count--; } 
         }                            
 return(nboct);
}

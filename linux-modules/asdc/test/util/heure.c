/*********************************************************/
/*        Mise en conformite gcc v4 le 15 fevrier 2008   */
/*********************************************************/

#include "config.h"

#ifdef PC_MSDOS

#include <dos.h>
#include "noms_ext.h"

char *heure()
{
  union REGS rin, rout;

  static char vheure[9];

  rin.h.ah = 0x2C;
  int86(0x21, &rin, &rout);
  sprintf(vheure,"%2d:%2d:%2d",rout.h.ch,rout.h.cl,rout.h.dh);

  if (vheure[3]==' ') vheure[3]='0';
  if (vheure[6]==' ') vheure[6]='0';

  return(vheure);
}



#endif   /* PC_MSDOS   */



#ifdef SUN_BSD

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include "noms_ext.h"

char *heure()
{
  static char vheure[9];
  
  struct tm *t;
  time_t horloge;
  
  horloge = time(0);
  t = localtime(&horloge);

  sprintf(vheure,"%2d:%2d:%2d" ,t->tm_hour, t->tm_min, t->tm_sec);

  if (vheure[3]==' ') vheure[3]='0';
  if (vheure[6]==' ') vheure[6]='0';

  return(vheure);
}


#endif   /* SUN_BSD   */

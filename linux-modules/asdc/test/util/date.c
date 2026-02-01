/*********************************************************/
/*   Extraction et mise en forme de la date du systeme   */
/*                                                       */
/*                                 Anonymized          */
/*                   derniere modif. le 8 janvier 1991   */
/*                                                       */
/*        Mise en conformite gcc v4 le 15 fevrier 2008   */
/*********************************************************/

#include "config.h"


#ifdef PC_MSDOS

#include <dos.h>
#include "noms_ext.h"

char *date(typ)
int typ;     /* si type = 0 : date = "num_jour/num_mois/an"
                          1 : date = "num_jour nom_mois an"
                          2 : date = "nom_jour num_jour nom_mois an" */
{
  union REGS rin, rout;

  static char vdate[35];
  static char *jour[] = { "dimanche", "lundi", "mardi", "mercredi",
                          "jeudi", "vendredi", "samedi", "dimanche" };
  static char *mois[] = { "janvier", "fevrier", "mars", "avril", "mai",
                          "juin", "juillet", "aout", "septembre",
                          "octobre","novembre", "decembre" };

  rin.h.ah = 0x2A;
  int86(0x21, &rin, &rout);

  if ((typ<0) || (typ>2)) typ=0;

  switch (typ)
  { case 0 : sprintf(vdate,"%d/%d/%d",
                     rout.h.dl,rout.h.dh,rout.x.cx);
             break;

    case 1 : sprintf(vdate,"%d %s %d",
                     rout.h.dl,mois[rout.h.dh-1],rout.x.cx);
             break;

    case 2 : sprintf(vdate,"%s %d %s %d",
                     jour[rout.h.al],rout.h.dl,
                     mois[rout.h.dh-1],rout.x.cx);
  }

  return(vdate);
}


#endif   /* PC_MSDOS  */


#ifdef SUN_BSD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include "noms_ext.h"

char *date(typ)
int typ;     /* si type = 0 : date = "num_jour/num_mois/an"
                          1 : date = "num_jour nom_mois an"
                          2 : date = "nom_jour num_jour nom_mois an" */
{
  struct tm *t;

  static char vdate[35];
  static char *jour[] = { "dimanche", "lundi", "mardi", "mercredi",
                          "jeudi", "vendredi", "samedi", "dimanche" };
  static char *mois[] = { "janvier", "fevrier", "mars", "avril", "mai",
                          "juin", "juillet", "aout", "septembre",
                          "octobre","novembre", "decembre" };
  time_t horloge;
  
  horloge = time(0);
  t = localtime(&horloge);

  if ((typ<0) || (typ>2)) typ=0;

  switch (typ)
  { case 0 : sprintf(vdate,"%d/%d/%d",
                     t->tm_mday, t->tm_mon+1, t->tm_year);
             break;

    case 1 : sprintf(vdate,"%d %s %d",
                     t->tm_mday,mois[t->tm_mon], 1900+t->tm_year);
             break;

    case 2 : sprintf(vdate,"%s %d %s %d",
                     jour[t->tm_wday], t->tm_mday,
                     mois[t->tm_mon], 1900+t->tm_year);
  }

  return(vdate);
}


#endif   /* SUN_BSD  */

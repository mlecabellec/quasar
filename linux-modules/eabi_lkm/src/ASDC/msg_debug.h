
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

Macros et prototypes permettant l'ecriture de messages brefs dans un tampon
en memoire afin de permettre de debugger les parties dynamiques du driver
qui fonctionnent en temps reel.


 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 8/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/


#ifndef MSG_DEBUG_H_
#define MSG_DEBUG_H_




# include "driverIncludes.h"


/*** Fonctions de debug ***********************************/
/* Ecriture dans le tampon des messages                   */
/* Le tampon des messages est relu par ioctl(ASDCLMSG)    */
/*   ==> cf. l'utilitaire lmsg.c dans ./test              */
/**********************************************************/

/* Ecriture chaine de caracteres */
void asdcmsg_ch(struct asdc_varg *dst, char *x);

/* Ecriture valeur numerique decimale */
void asdcmsg_d(struct asdc_varg *dst, int x);

/* Ecriture valeur numerique hexadecimale */
void asdcmsg_x(struct asdc_varg *dst, int x);

/* Ecriture indicateur fin message */
void asdcmsg_fin(struct asdc_varg *dst);


/*** Quelques macros pour faciliter l'appel des fonctions ci-dessus : ***/

/* Macros pour enregistrer dans le tampon :      */
/*    MSG    : Une chaine de caracteres          */
/*    MSGD   : Une valeur numerique decimale     */
/*    MSGX   : Une valeur numerique hexadecimale */
/*    MSGFIN : Un indicateur de fin de smessages */
#define MSG(x)          asdcmsg_ch(dst, (x))
#define MSGD(x)         asdcmsg_d(dst, (x))
#define MSGX(x)         asdcmsg_x(dst, (x))
#define MSGFIN          asdcmsg_fin(dst)




#endif   /* MSG_DEBUG_H_ */


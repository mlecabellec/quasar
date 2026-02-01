
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

Fonctions pour ecriture rapide de messages dans un tampon afin de
permettre un debug du driver aussi peu intrusif que possible.

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 8/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/


#ifndef BUILDING_FOR_KERNEL
#define BUILDING_FOR_KERNEL     
#endif


#include "driverIncludes.h"


/*** Fonctions de debug ***********************************/
/* Ecriture dans le tampon des messages                   */
/* Le tampon des messages est relu par ioctl(ASDCLMSG)    */
/*   ==> cf. l'utilitaire lmsg.c dans ./test              */

/* Ecriture chaine de caracteres */
void asdcmsg_ch(struct asdc_varg *dst, char *x)
{
  int n;
  char *p;
  unsigned long s;    /* Pour masquage IT pendant sections critiques */

  // kkprintf("asdcmsg_ch: dst=0x%04X\n", dst);
  // kkprintf("    ch=\"%s\"\n", x);

  /* Taille chaine */
  n = 0;
  for (p=x; *p; p++) n++;

  // kkprintf("   n=%d\n", n);

  /* Pour permettre appel depuis fonction d'IT */
  spin_lock_irqsave(&dst->lock_debug, s);

    /* Test debordement */
    if (dst->msg.ic > (TTMSG - (n+2))) {
      spin_unlock_irqrestore(&dst->lock_debug, s);
      return;
    }

    /* Ecriture */
    dst->msg.ch[dst->msg.ic++] = 1;   // code "chaine"
    for (p=x; *p; p++) dst->msg.ch[dst->msg.ic++] = *p;
    dst->msg.ch[dst->msg.ic++] = 0;

  spin_unlock_irqrestore(&dst->lock_debug, s);
}

/* Ecriture valeur numerique decimale */
void asdcmsg_d(struct asdc_varg *dst, int x)
{
  int n, i;
  char *p;
  unsigned long s;    /* Pour masquage IT pendant sections critiques */

  /* Taille objet */
  n = 4;    // sizeof(int)

  /* Pour permettre appel depuis fonction d'IT */
  spin_lock_irqsave(&dst->lock_debug, s);

    /* Test debordement */
    if (dst->msg.ic > (TTMSG - (n+1))) {
      spin_unlock_irqrestore(&dst->lock_debug, s);
      return;
    }

    /* Ecriture */
    dst->msg.ch[dst->msg.ic++] = 2;   // code "entier decimal"
    for (p=(char *) &x, i=0; i<4; p++, i++) dst->msg.ch[dst->msg.ic++] = *p;

  spin_unlock_irqrestore(&dst->lock_debug, s);
}

/* Ecriture valeur numerique hexadecimale */
void asdcmsg_x(struct asdc_varg *dst, int x)
{
  int n, i;
  char *p;
  unsigned long s;    /* Pour masquage IT pendant sections critiques */

  /* Taille objet */
  n = 4;    // sizeof(int)

  /* Pour permettre appel depuis fonction d'IT */
  spin_lock_irqsave(&dst->lock_debug, s);

    /* Test debordement */
    if (dst->msg.ic > (TTMSG - (n+1))) {
      spin_unlock_irqrestore(&dst->lock_debug, s);
      return;
    }

    /* Ecriture */
    dst->msg.ch[dst->msg.ic++] = 3;   // code "entier hexadecimal"
    for (p=(char *) &x, i=0; i<4; p++, i++) dst->msg.ch[dst->msg.ic++] = *p;

  spin_unlock_irqrestore(&dst->lock_debug, s);
}

/* Ecriture indicateur fin message */
void asdcmsg_fin(struct asdc_varg *dst)
{
  unsigned long s;    /* Pour masquage IT pendant sections critiques */

  /* Pour permettre appel depuis fonction d'IT */
  spin_lock_irqsave(&dst->lock_debug, s);

    /* Test debordement */
    if (dst->msg.ic > (TTMSG - (1))) {
      spin_unlock_irqrestore(&dst->lock_debug, s);
      return;
    }

    /* Ecriture */
    dst->msg.ch[dst->msg.ic++] = 4;   // code "fin message"

  spin_unlock_irqrestore(&dst->lock_debug, s);
}

/*** Fin des fonctions de debug ***************************/



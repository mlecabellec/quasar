
/*

 DRIVER EMUABI

Ce fichier contient les definitions de macros et de fonctions inline
permettant l'acces a la memoire d'echange et a la memoire image depuis
les fonctions do_XXXX qui implementent les commandes ioctl du driver.


 QUAND      QUI   QUOI
---------- ----  --------------------------------------------------------------
 8/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
 9/04/2013  YG   Ajout des prototypes des fonctions definies dans do_acces.c.
 3/06/2013  YG   Suppression des acces a un registre CSR obsolete.
11/02/2015  YG   Definition EX(), LX() et LLX().



*/


#ifndef EMUABI_MACROS_H
#define EMUABI_MACROS_H




/****************************************************************
 ***   Macros pour acceder aux mots de la memoire d'echange   ***
 ***   et de son "image"                                      ***
 ****************************************************************/

/*
 * Les macros E et L ont pour but, outre la facilite d'ecriture,
 * l'augmentation de la lisibilite, de limiter les modifications
 * du code le jour ou l'acces a la memoire d'echange via les fonctions
 * asdc_lire() et asdc_ecrire() sera remplace par un acces direct
 * (avec la conversion hard petit/grand endian validee sur le
 * coupleur PMC ABI/ASF).
 */

/* Ces macros ne peuvent etre appelees que dans une fonction du   */
/* noyau ayant pour argument de nom "dst" un pointeur vers les    */
/* donnees statiques du driver.                                   */
#ifdef FONCTION_INTERRUPTION
#  define L(a)            asdc_lire_ram(dst, 1, (a), dst->pcibar1, 0, dst->vflash)
#  define LL(a, p)        asdc_lire_ram(dst, 1, (a), dst->pcibar1, p, dst->vflash)
#  define E(a, v)         asdc_ecrire_ram(dst, 1, (a), dst->pcibar1, (v), dst->vflash)
#else
#  define L(a)            asdc_lire_ram(dst, 0, (a), dst->pcibar1, 0, dst->vflash)
#  define LL(a, p)        asdc_lire_ram(dst, 0, (a), dst->pcibar1, p, dst->vflash)
#  define E(a, v)         asdc_ecrire_ram(dst, 0, (a), dst->pcibar1, (v), dst->vflash)
#  define LX(a)           asdc_lire(dst, 0, (a), dst->pcibar1, 0, dst->vflash)
#  define LLX(a, p)       asdc_lire(dst, 0, (a), dst->pcibar1, p, dst->vflash)
#  define EX(a, v)        asdc_ecrire(dst, 0, (a), dst->pcibar1, (v), dst->vflash)
#endif
#define LI(a, p)        (dst->image[asdc_indice(a, 'L', p)])
#define EI(a, v, p)     (dst->image[asdc_indice(a, 'E', p)] = (v))


/* Ces 4 macros permettent d'acceder aux 16 bits de poids fort et   */
/* faible de chacun des mots de la memoire image                    */
/*       ATTENTION a l'endianite !!!                                */
#ifdef GROS_BOUT

#define LIH(a, p)       \
        (*((uint16 *) &dst->image[asdc_indice(a, 'L', p)]))
#define EIH(a, v, p)    \
        (*((uint16 *) &dst->image[asdc_indice(a, 'E', p)]) = (v))
#define LIL(a, p)       \
        (*((uint16 *) &dst->image[asdc_indice(a, 'L', p)] + 1))
#define EIL(a, v, p)    \
        (*((uint16 *) &dst->image[asdc_indice(a, 'E', p)] + 1) = (v))

#else

#define LIL(a, p)       \
        (*((uint16 *) &dst->image[asdc_indice(a, 'L', p)]))
#define EIL(a, v, p)    \
        (*((uint16 *) &dst->image[asdc_indice(a, 'E', p)]) = (v))
#define LIH(a, p)       \
        (*((uint16 *) &dst->image[asdc_indice(a, 'L', p)] + 1))
#define EIH(a, v, p)    \
        (*((uint16 *) &dst->image[asdc_indice(a, 'E', p)] + 1) = (v))

#endif   /* GROS_BOUT */

/* L'argument p de EI et LI est un marqueur permettant d'identifier */
/* chaque appel (a condition de lui donner des valeurs chaque fois  */
/* differentes). Cette identification est passee a asdc_indice pour */
/* affichage en cas de detection d'une adresse hors bornes.         */




/* Prototypes des fonctions utilisees par les macros ci-dessus */
// Ces fonctions sont maintenant inline ==> Ces prototypes ne sont plus utiles
// unsigned short asdc_lire(int adresse_mot, char * base_octet,
//                                            int position, int flash);
// void asdc_ecrire(int adresse_mot, char * base_octet,
//                                            short valeur, int flash);
// int asdc_indice(int adresse_mot, char op, int pos);


/*******************************************************************
 ***   Fonctions pour acceder aux mots de la memoire d'echange   ***
 *******************************************************************/

union echange_octets
 {
   short mot;
   char octet[2];
 };

// Pour fonctionner en big endian, decommenter la ligne ci-dessous
//#define GROS_BOUT

union swap_zone { 
    unsigned long l;
    struct {
        char x1;
        char x2;
        char x3;
        char x4;
    } b;
};

inline static void swap_mot(unsigned int *mot)
{
#ifdef GROS_BOUT
    char tmp;
    union swap_zone u;
    u.l = *mot;
    tmp = u.b.x1;
    u.b.x1 = u.b.x4;
    u.b.x4 = tmp;
    tmp = u.b.x2;
    u.b.x2 = u.b.x3;
    u.b.x3 = tmp;
    *mot = u.l;
#endif
}


inline static unsigned int
asdc_lire(struct asdc_varg * dst, int it,
          int adresse_mot, void * base_octet, int position, int vflash)
{ 
  void * adata;
  unsigned int vdata;

  /* Pour traquer les bugs en detectant les acces effectues en dehors de la */
  /* memoire d'echange.                                                     */
  /* vflash : positionne dans l'utilitaire de flashage des cartes SBS. En   */
  /* effet, cet utilitaire effectuait de facon normal des acces en dehors   */
  /* des bornes de la memoire d'echange.                                    */
  if (!vflash && ((adresse_mot < 0) && (adresse_mot >= FIN_RAM)))
    {
      kkprintf("asdc_lire(0x%X)   Position=%d\n", adresse_mot, position);
    }
  
  adata = base_octet + (adresse_mot * 4);
  vdata = lecturePCIe(dst->numVfx, it, adata);
  swap_mot(&vdata);


  /* cprintf("Lecture 0x%X a l'adresse 0x%X\n", eo.mot, adresse_mot); */

  return vdata;
}

inline static unsigned int
asdc_lire_ram(struct asdc_varg * dst, int it,
              int adresse_mot, void * base_octet, int position, int vflash)
{ 
  unsigned int vdata;

  /* Pour traquer les bugs en detectant les acces effectues en dehors de la */
  /* memoire d'echange.                                                     */
  /* vflash : positionne dans l'utilitaire de flashage des cartes SBS. En   */
  /* effet, cet utilitaire effectuait de facon normal des acces en dehors   */
  /* des bornes de la memoire d'echange.                                    */
  if (!vflash && ((adresse_mot < 0) && (adresse_mot >= FIN_RAM)))
    {
      kkprintf("asdc_lire(0x%X)   Position=%d\n", adresse_mot, position);
    }
  
  vdata = lectureRamPCIe(dst->numVfx, it, adresse_mot);
  swap_mot(&vdata);


  /* cprintf("Lecture 0x%X a l'adresse 0x%X\n", eo.mot, adresse_mot); */

  return vdata;
}

inline static void
asdc_ecrire(struct asdc_varg * dst, int it,
            int adresse_mot, void * base_octet, int valeur, int vflash)
{ 
  void * adata;
  unsigned int vdata;

  /* cprintf("Ecriture 0x%X a l'adresse 0x%X\n", valeur, adresse_mot); */


  /* Pour traquer les bugs en detectant les acces effectues en dehors de la */
  /* memoire d'echange.                                                     */
  /* vflash : positionne dans l'utilitaire de flashage des cartes SBS. En   */
  /* effet, cet utilitaire effectuait de facon normal des acces en dehors   */
  /* des bornes de la memoire d'echange.                                    */
  if (!vflash && ((adresse_mot < 0) && (adresse_mot >= FIN_RAM)))
    {
      kkprintf("asdc_ecrire(0x%X, 0x%X)\n", adresse_mot, valeur & 0xFFFF);
    }

  vdata = valeur;
  swap_mot(&vdata);
  adata = base_octet + (adresse_mot * 4);
  ecriturePCIe(dst->numVfx, it, vdata, adata);
}

inline static void
asdc_ecrire_ram(struct asdc_varg * dst, int it,
                int adresse_mot, void * base_octet, int valeur, int vflash)
{ 
  unsigned int vdata;

  /* cprintf("Ecriture 0x%X a l'adresse 0x%X\n", valeur, adresse_mot); */


  /* Pour traquer les bugs en detectant les acces effectues en dehors de la */
  /* memoire d'echange.                                                     */
  /* vflash : positionne dans l'utilitaire de flashage des cartes SBS. En   */
  /* effet, cet utilitaire effectuait de facon normal des acces en dehors   */
  /* des bornes de la memoire d'echange.                                    */
  if (!vflash && ((adresse_mot < 0) && (adresse_mot >= FIN_RAM)))
    {
      kkprintf("asdc_ecrire(0x%X, 0x%X)\n", adresse_mot, valeur & 0xFFFF);
    }

  vdata = valeur;
  swap_mot(&vdata);
  ecritureRamPCIe(dst->numVfx, it, vdata, adresse_mot);
}


/* Pour traquer les bugs (debordement du tableau image[])     */
/*      adresse_mot : valeur dont les bornes sont surveillees */
/*      op : 'L' ou 'E' selon appel dans macro LI ou EI       */
/*      pos : marqueur permettant d'identifier l'appel fautif */
inline static int
asdc_indice(int adresse_mot, char op, int pos)
{
  if (    (adresse_mot < 0)
       || (adresse_mot > 65535)
     )
    {
      kkprintf("asdc_indice(0x%X, %c, %d)\n", adresse_mot, op, pos);
    }

  return adresse_mot;
}


inline static int
asdc_lecture_brute(void * base, long adresse)
{
  void * adata;
  unsigned int vdata;

  adata = base + adresse;
  vdata = readl(adata);
  swap_mot(&vdata);

  return vdata;
}


inline static void
asdc_ecriture_brute(void * base, long adresse, int valeur)
{
  void * adata;
  unsigned int vdata;

  vdata = valeur;
  swap_mot(&vdata);
  adata = base + adresse;
  writel(vdata, adata);
}



/*************************************************************/
/***   Prototypes des fonctions definies dans do_acces.c   ***/
/*************************************************************/
int asdc_sleep(struct asdc_varg *dst, int duree_ms);


#endif   /* EMUABI_MACROS_H */

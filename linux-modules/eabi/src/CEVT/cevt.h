/************************************************************************
 *                                                                      *
 *     Pseudo-driver "Concentrateur d'EVenemenTs" pour ASDC et ETOR     *
 *     ------------------------------------------------------------     *
 *                                                                      *
 *         Definition des registres materiels et des constantes         *
 *                                                                      *
 *                                                                      *
 *                                         Y.Anonymized, le  9/09/2002   *
 *            (XS ?) : Declaration cevt_signaler_date() le 22/05/2003   *
 *                                   Adaptation a Linux le 18/10/2006   *
 *                       Adaptation au TSC (sous Linux) le 25/10/2006   *
 *              Modif. types pour compatibilite 32b/64b le 18/06/2014   *
 *                                                                      *
 * Fusion des drivers VFX70, ASDC et CEVT en un                         *
 *                                    seul module : YG, le  8/01/2015   *
 ************************************************************************/



#if !defined( _CEVT_H )
#define _CEVT_H

#include <linux/types.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/uaccess.h>
#endif

// #ifndef _LINUX_IOCTL_H
// #include <linux/ioctl.h>
// #endif





/* Nombre maximum de devices CEVT */
#define CEVT_MAX_DEVICES	25



/* table des structures statiques de chaque device */
extern struct cevt_statics * cevt_ps[CEVT_MAX_DEVICES];

/* Prototype de la fonction ioctl du driver */
long cevtioctl(int vfxNum, int asdcNum, struct file *fp,
               unsigned int cmd, unsigned long arg);


/* Fonctions d'acces direct (par les autres drivers) au pseudo-driver CEVT */

/* Renvoi 0 si CEVT dont numero passe en arg. n'existe pas */
int cevt_existence_impl(int numero_cevt);

int cevt_signaler_impl(
            int     numero_cevt,
            int     nature_es,     /* CEVT_ETOR, CEVT_1553, etc... */
            int     numero_es,     /* numero voie/coupleur */
            int     numero_bus,    /* numero voie/coupleur */
            int32_t evenement,     /* CEVT_AVORTE ou descr. evenement */
            int32_t donnee         /* Donnee associee eventuelle */
            );

int cevt_signaler_date_impl(
            int     numero_cevt,
            int     nature_es,  /* CEVT_ETOR, CEVT_1553, etc... */
            int     numero_es,  /* numero voie/coupleur */
            int     numero_bus, /* numero voie/coupleur */
            int32_t evenement,  /* CEVT_AVORTE ou descr. evenement */
            int32_t donnee,     /* Donnee associee eventuelle */
            unsigned long long tsc /* date */
            );




#endif /* !defined( _CEVT_H ) */

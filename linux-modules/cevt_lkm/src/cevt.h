/************************************************************************
 *                                                                      *
 *     Pseudo-driver "Concentrateur d'EVenemenTs" pour ASDC et ETOR     *
 *     ------------------------------------------------------------     *
 *                                                                      *
 *         Definition des registres materiels et des constantes         *
 *                                                                      *
 *                                                                      *
 *                                         Y.Guillemot, le  9/09/2002   *
 *            (XS ?) : Declaration cevt_signaler_date() le 22/05/2003   *
 *                                   Adaptation a Linux le 18/10/2006   *
 *                       Adaptation au TSC (sous Linux) le 25/10/2006   *
 *                                                                      *
 *                                      derniere modif. le              *
 ************************************************************************/



#if !defined( _CEVT_H )
#define _CEVT_H


#ifdef LYNXOS

#ifndef _IOCTL_
#include <sys/ioctl.h>
#endif

#elif LINUX

#ifndef _LINUX_IOCTL_H
#include <linux/ioctl.h>
#endif


#endif /* LYNXOS - LINUX */



/* Nombre maximum de devices CEVT */
#define CEVT_MAX_DEVICES	25



/* Structure tampon pour flux evenements */
struct cevt_tf { int type;		/* Type de l'evenement */
		 int src;		/* Source de l'evenement (voie) */
                 int d1;		/* Donnee 1 (cmd 1553) */
                 int d2;		/* Donnee 2 (donee CC eventuelle) */
                 union { long date[2];	/* Datation (non encore utilise) */ 
                         unsigned long long tsc;   /* Pour Linux */
                       };
               };
		
/* Structure de chainage des tampons pour flux_evt RT */
struct cevt_tfch { struct cevt_tf tamp;	/* Tampon */
		   struct cevt_tfch *s;	/* Pointeur du suivant */
		 };








#endif /* !defined( _CEVT_H ) */

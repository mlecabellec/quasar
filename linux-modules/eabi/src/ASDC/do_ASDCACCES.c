
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCACCES : Fonction ioctl d'acces a un mot de la memoire physique du coupleur.

 QUAND      QUI   QUOI
---------- ----  --------------------------------------------------------------
15/04/2013  YG   Version initiale.
 7/05/2013  YG   Suppression d'un message de debug un peu trop present...
 3/06/2013  YG   Suppression des declarations de variables inutilisees...
13/06/2014  YG   Rassemblement des includes dans un seul fichier
11/02/2015  YG   Acces mem. ech. sont fait avec LX() et EX()
                 Acces par L() et E() conserves pour validation (ASDC_MEMESS)
26/02/2015  YG   Interversion de L() et LX() d'une part, E() et EX() d'autre
                 part, depuis que l'acces purement materiel a la RAM via son
                 "port droit" est fonctionnel.

*/



#include "driverIncludes.h"



/// TODO : Seuls arguments sans doute necessaires : dst et arg !!!

long
do_ASDCACCES(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    struct asdcacces *a, za;

    /* Definition d'une "voie 1553" */
    // printk("asdcioctl.c entree dans case ASDCDEF \n"); //ajout
    TDUTIL(arg, sizeof(struct asdcacces));
    DUTIL(za, a, arg, sizeof(struct asdcacces));
    TVUTIL(za, a, arg, sizeof(struct asdcacces));


    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, on ne fait rien...                   */


    /* a->zone : ASDC_PCIBAR0, ASDC_PCIBAR1, ASDC_PCIBAR2, ASDC_MEM   */
    /*           ou ASDC_IMAGE                                        */
    /* a->op : ASDC_LIREMOT ou ASDC_ECRIREMOT                         */
    /* a->adresse : adresse concernee, dans la zone specifiee         */
    /* a->valeur : mot a ecrire ou relu                               */


  /*   POUR DEBUG SEULEMENT
    switch(a->op) {
        case ASDC_LIREMOT :   op = "LIRE";      break;
        case ASDC_ECRIREMOT : op = "ECRIRE";    break;
        default :             op = "INCONNUE";  break;
    }
    switch(a->zone) {
        case ASDC_PCIBAR0 :   zone = "PCI_BAR0";  break;
        case ASDC_PCIBAR1 :   zone = "PCI_BAR1";  break;
        case ASDC_PCIBAR2 :   zone = "PCI_BAR2";  break;
        case ASDC_MEM :       zone = "MEMECH";    break;
        case ASDC_MEMESS :    zone = "MEMECH_ESS";    break;
        case ASDC_IMAGE :     zone = "IMAGE";     break;
        default :             zone = "INCONNUE";  break;
    }
    printk("%s %s @=0x%lX\n", op, zone, a->adresse);
  */

    switch(a->op) {
        case ASDC_LIREMOT :
            switch(a->zone) {
                case ASDC_PCIBAR0 :
                    a->valeur = asdc_lecture_brute(dst->pcibar0, a->adresse);
                break;
                case ASDC_PCIBAR1 :
                    a->valeur = asdc_lecture_brute(dst->pcibar1, a->adresse);
                break;
                case ASDC_PCIBAR2 :
                    a->valeur = asdc_lecture_brute(dst->pcibar2, a->adresse);
                break;
                case ASDC_MEM :
                    a->valeur = L(a->adresse);
                break;
                case ASDC_MEMESS :   /* Pour essai de validation */
                    a->valeur = LX(a->adresse);
                break;
                case ASDC_IMAGE :
                    a->valeur = LI(a->adresse, 0);
                break;
                default :
                    RETURN(ENXIO);
            }
        break;
        case ASDC_ECRIREMOT :
            switch(a->zone) {
                case ASDC_PCIBAR0 :
                    asdc_ecriture_brute(dst->pcibar0, a->adresse, a->valeur);
                break;
                case ASDC_PCIBAR1 :
                    asdc_ecriture_brute(dst->pcibar1, a->adresse, a->valeur);
                break;
                case ASDC_PCIBAR2 :
                    asdc_ecriture_brute(dst->pcibar2, a->adresse, a->valeur);
                break;
                case ASDC_MEM :
                     E(a->adresse, a->valeur);
                break;
                case ASDC_MEMESS :   /* Pour essai de validation */
                     EX(a->adresse, a->valeur);
                break;
                case ASDC_IMAGE :
                    EI(a->adresse, a->valeur, 0);
                break;
                default :
                    RETURN(ENXIO);
            }
        break;
        default :
           RETURN(ENOTTY);
    }

    /* Retour des donnees a l'appli appelante */
    VUTIL(za, a, arg, sizeof(struct asdcacces));
    RETURN(OK);
}


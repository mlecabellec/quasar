/****************************************************************/
/* QABI : Resource manager QNX pour ABI-PMC2-2/IRIG             */
/*                                                              */
/* Interface io_devctl() : Rebouclage d'une "voie" en reception */
/* d'un abonne vers une "voie" en emission du meme abonne.      */
/*                                                              */
/*                          ASTRIUM ST - TE641 - Yves Guillemot */
/****************************************************************/

/*
    Une "voie" est constituee par le couple adresse + sous-adresse.

    Une fois le "rebouclage" effectue, un seul tampon est utilise et les deux
    sous-adresses (emission et reception) pointent vers ce tampon.
    
    Pour que le rebouclage (ou le lien) soit possible :
       - La sous-adresse en emission doit etre definie, en mode STATIQUE, et
         avec un seul tampon.
       - La sous-adresse en reception ne doit pas etre definie.
*/

/*
Quand       Qui   Quoi
----------  ----  -------------------------------------------------------------
10/05/2010  yg    - Version initiale
 1/06/2010  yg    - Utilisation typedef abiStatics_t pour la structure statics
                    du coupleur 1553
 9/10/2013  yg    - Adaptation a Linux
11/06/2014  yg    - Adaptation au coupleur EMUABI en reintroduisant du code
                    en provenance des driver et LN2 ASDC (CAMBUS et CMB).
13/06/2014  YG   Rassemblement des includes dans un seul fichier
17/06/2014  YG   Correction bugs sur determination zd (Zone des Donnees)
29/01/2015  YG   Utilisation d'un pointeur tampon suivant = -1 pour les voies
                 en emission en mode asynchrone.
*/



#include "driverIncludes.h"



/* Parametrage des elements de la fonction :                                */
/*    DEVCTL_NOM est un nom pour affichage au debug uniquement              */
/*    STRUCT_ES est la structure E/S (peut-etre une union)                  */
/*    STRUCT_E est la structure d'entree (peut etre identique a STRUCT_ES)  */
/*    STRUCT_S est la structure de sortie (peut etre identique a STRUCT_ES) */
#define DEVCTL_NOM     "ASDC_SA_LIER"
#define STRUCT_E       abiboucl_t



long
do_ASDC_SA_LIER(int vfxNum, int asdcNum,
                 struct asdc_varg *dst, unsigned long arg)
{

    /* Structure d'E/S et pointeur associe */
    STRUCT_E data;
    STRUCT_E *ptr;

    /* Variables specifiques a cette fonction io_devctl particuliere */
    int atptr, swtptr;
    int bus, adr, sar, sat;
    int saTable, pSaTable;
    int pTamponR, aTamponR;
    int pTamponT, aTamponT;
    int espion_tr;
    int zdr, zdt;  /* Adresses des "Zones de donnees" des deux voies */
    int pe;

    /* Message de debug eventuel */
//     printLog(ABI_DEBUG, "%s\n", DEVCTL_NOM);

    /* Copie des donnees depuis l'espace utilisateur */
    if (copy_from_user(&data, (STRUCT_E *) arg, sizeof(data))) return -EFAULT;
    ptr = &data;

    /******************************/
    /** Debut du code specifique **/

    /* Adresse 1553, sous-adresse en reception et sous-adresse en emission */
    bus = ptr->bus & 0x01;
    adr = ptr->adr & 0x1F;
    sar = ptr->sar & 0x1F;
    sat = ptr->sat & 0x1F;

//     printLog(ABI_DEBUG, "do_abi_sareboucler bus=%d adr=%d sar=%d sat=%d\n",
//                          bus, adr, sar, sat);
    
    /* Adresses des tables */
    atptr  = L(ATPTR);
    swtptr = L(SWTPTR);
  
    /* Les tables sont elles bien definies ? */
    if ((atptr == 0) || (swtptr == 0)) { 
        return -EADDRNOTAVAIL;      /* L'une des tables n'est pas definie ! */
    }

    /* Recherche adresses des tampons */
    pSaTable = atptr + adr + 32 * bus; /* Adresse ptr table des sous-adresses */
    saTable = L(pSaTable);             /* Adresse table des sous-adresses */
//     printLog(ABI_DEBUG, "saTable : 0x%04X\n", saTable);
    if (saTable == 0) {
//         printLog(ABI_ERROR, "RT%d n'est pas defini !\n", adr);
       return -EADDRNOTAVAIL;
    }

    pTamponR = saTable + 0 + sar;
    aTamponR = L(pTamponR);
//     printLog(ABI_DEBUG, "pTamponR=0x%04X aTamponR=0x%04X\n",
//                          pTamponR, aTamponR);

    zdr = LI(saTable + sar, -1);   /* Zone des donnees voie R */

    if ((aTamponR != 0) || (zdr != 0)) {
//         printLog(ABI_ERROR,
//                  "La voie en reception RT%d,%d est deja definie !\n",
//                  adr, sar);
        return -EADDRINUSE;      /* La voie R est deja definie ! */
    }

    pTamponT = saTable + 32 + sat;
    aTamponT = L(pTamponT);
//     printLog(ABI_DEBUG, "pTamponT=0x%04X aTamponT=0x%04X\n",
//                          pTamponT, aTamponT);
    
    zdt = LI(saTable + 32 + sat, -1);   /* Zone des donnees voie T */
//     printk("adr=%d sat=%d aTamponT=0x%X zdt=0x%X\n", adr, sat, aTamponT, zdt);
    
    if ((aTamponT == 0 || zdt == 0)) {
//         printLog(ABI_ERROR,
//                  "La voie en emission RT%d,%d n'est pas definie !\n",
//                  adr, sat);
        return -EADDRNOTAVAIL;      /* La voie T n'est pas definie ! */
    }

    /* RT programme en "espion TR" ? */
    espion_tr = L(L(PROPTR) + adr + 32 * bus) & 2;


    /* Controle de l'unicite du tampon T */
    pe = L(aTamponT);
    if ((pe != aTamponT) && (pe != 0xFFFFFFFF)) {
//         printk("aTamponT=0x%0X  pe=0x%0X\n", aTamponT, pe);
//             printLog(ABI_ERROR,
//                     "La voie en emission RT%d,%d a plusieurs tampons !\n",
//                     adr, sat);
            return -EMLINK;      /* Le tampon T n'est pas unique ! */
    }
    
    /* Mise en commun du tampon restant */
    E(pTamponR, aTamponT);
    E(aTamponT, aTamponT);   /* Et rebouclage au cas ou... */
    
    /* et des donnees de la memoire image */
    EI(saTable + sar, zdt, -1);
    
    /* et programmation de la SA en reception avec le meme nombre de */
    /* mots legal que la SA en emission (le nombre de mots legal est */
    /* range dans la table des SA, 64 mots au dela du pointeur des   */
    /* tampons).                                                     */
    E(pTamponR + 64, L(pTamponT + 64));
    
    /* REMARQUE : Une voie en reception liee peut etre identifiee   */
    /*            en comparant le mot de commande stocke en memoire */  
    /*            image (IRCMD) au sens de la voie.                 */
  




//     printLog(ABI_DEBUG, "Rebouclage RT%d,%d et RT%d,%d\n",
//                          adr, sar, adr, sat);



    /******************************************************************/
    /** Debut du code generique de preparation du renvoi des donnees **/


    /** Fin du code generique de preparation du renvoi des donnees **/
    /****************************************************************/

    /* Recopie des donnees a transferer */
        /* Aucune donnee a transferer !!! */

    /** Fin du code specifique **/
    /****************************/

    return -EOK;
}





/****************************************************************/
/* QABI : Resource manager QNX pour ABI-PMC2-2/IRIG             */
/*                                                              */
/* Interface io_devctl() : Debouclage de deux "voies" d'un      */
/* abonne, une en reception et une en emission, qui ont ete     */
/* "rebouclees" pour permettre un test de type "SA30".          */
/*                                                              */
/*                          ASTRIUM ST - TE641 - Yves Guillemot */
/****************************************************************/

/*
    Une "voie" est constituee par le couple adresse + sous-adresse.

    Une fois le "rebouclage" (ou "lien") effectue, un seul tampon est utilise
    et les deux sous-adresses (emission et reception) pointent vers ce tampon.

    Si le le debouclage (ou la rupture du lien) est effectuee :
       - La sous-adresse en emission reste definie et conserve le tampon.
       - La sous-adresse en reception devient non definie.
*/

/*
Quand       Qui   Quoi
----------  ----  -------------------------------------------------------------
11/05/2010  yg    - Version initiale
 1/06/2010  yg    - Utilisation typedef abiStatics_t pour la structure statics
                    du coupleur 1553
 9/10/2013  yg    - Adaptation a Linux
11/06/2014  yg    - Adaptation au coupleur EMUABI
13/06/2014  YG   Rassemblement des includes dans un seul fichier
17/06/2014  YG   Correction bugs sur determination zd (Zone des Donnees)

*/



#include "driverIncludes.h"




/* Parametrage des elements de la fonction :                                */
/*    DEVCTL_NOM est un nom pour affichage au debug uniquement              */
/*    STRUCT_ES est la structure E/S (peut-etre une union)                  */
/*    STRUCT_E est la structure d'entree (peut etre identique a STRUCT_ES)  */
/*    STRUCT_S est la structure de sortie (peut etre identique a STRUCT_ES) */
#define DEVCTL_NOM     "ASDC_SA_DELIER"
#define STRUCT_E       abiboucl_t



long
do_ASDC_SA_DELIER(int vfxNum, int asdcNum,
                  struct asdc_varg *dst, unsigned long arg)
{

    /* Structure d'E/S et pointeur associe */
    STRUCT_E data;
    STRUCT_E *ptr;

    /* Variables specifiques a cette fonction io_devctl particuliere */
    int bus, adr, sar;
    int saTable, pSaTable;
    int pTamponR, aTamponR;
    int espion_tr;
    int zdr;  /* Adresses de la "Zone de donnees" des la voie en reception */
    int atptr, swtptr;
    int cmd;

    /* Message de debug eventuel */
//     printLog(ABI_DEBUG, "%s\n", DEVCTL_NOM);

    /* Copie des donnees depuis l'espace utilisateur */
    if (copy_from_user(&data, (STRUCT_E *) arg, sizeof(data))) return -EFAULT;
    ptr = &data;

    /******************************/
    /** Debut du code specifique **/

    /* Adresse 1553, sous-adresse en reception (par convention, c'est cette */
    /* derniere seule qui est passee en argument).                          */
    bus = ptr->bus & 0x01;
    adr = ptr->adr & 0x1F;
    sar = ptr->sar & 0x1F;

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

    if ((aTamponR == 0) || (zdr == 0)) {
//         printLog(ABI_ERROR,
//                  "La voie en reception RT%d,%d n'est pas definie !\n",
//                  adr, sar);
        return -EADDRNOTAVAIL;      /* La voie R n'est pas definie ! */
    }

    /* On verifie qu'un rebouclage est bien effectue */
    cmd = LI(zdr + IRCMD, -1);
    if ((cmd & 0x0400) == 0) {
//         printLog(ABI_ERROR,
//                  "La voie "R" RT%d,%d n'est pas liee a une voie "T" !\n",
//                  adr, sar);
        return -ENOTBLK;      /* La voie R existe, mais n'est pas rebouclee */
    }

    /* RT programme en "espion TR" ? */
    espion_tr = L(L(PROPTR) + adr + 32 * bus) & 2;

//     /* Un debouclage ne peut pas etre effectue si l'abonne est */
//     /* en mode espion temps reel.                              */
//     if (espion_tr) return XXXX    // Et pourquoi donc ???
        
        
        
    /* Execution du debouclage */
    E(pTamponR, 0);              /* Suppression tampon R */
    EI(saTable + sar, 0, -1);    /* Suppression zd */
    E(pTamponR + 64, 0);         /* RAZ nombre mots legal (utile ???) */
    

//     printLog(ABI_DEBUG,
//             "Debouclage RT%d,%d\n",adr, sar);



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





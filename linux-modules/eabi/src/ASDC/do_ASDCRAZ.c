
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCRAZ : Fonction ioctl de (re)initialisation du coupleur EMUABI


 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 8/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
 9/04/2013  YG   Adaptation aux deux bus separes.
 2/05/2013  YG   Ajout flag "jamais_initialise".
12/06/2014  YG   Prise en compte de la FIFO associee aux sous-adresses en
                 emission.
13/06/2014  YG   Rassemblement des includes dans un seul fichier
11/02/2015  YG   Remplacement L() et E() par LX() et EX()
26/02/2015  YG   Le partage materiel du port droit de la SRAM est maintenant
                 fonctionnel. Le bon fonctionnement du powerPC n'etant plus
                 necessaire pour acceder a la RAM depuis le bus PCI, les macros
                 LX() et EX() peuvent de nouveau etre remplacees par L() et E().

*/



#include "driverIncludes.h"



/// TODO : Seuls arguments sanss doute necessaires : dst et arg !!!

long
do_ASDCRAZ(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{

    int i, j, k;

    /* DEBUT DE LA PARTIE SPECIFIQUE ASDCRAZ */



    printk("EMUABI : ASDCRAZ VFX70=%d ASDC=%d\n", vfxNum, asdcNum);
  
    /* Arret du micro-code */
    E(CMD, 0);
  
    /* Cette reinitialisation est-elle correcte ?                  */
    /* c.f. 53B-STANDARD MIL-STD-1553B PROGRAMMERS REFERENCE, p.16 */
  
    /* "Liberation" de la memoire allouee */
    asdcrazalloc(dst);
    /* Dans les quelques allocations qui suivent, on est sur de ne pas */
    /* saturer la memoire de l'AMI : la val. retournee par asdcalloc() */
    /* n'est donc pas testee                                           */
  
    /* --- Reinitialisation des pointeurs --- */
  
    /* Initialisation des queues d'interruption */
    E(IQRSP, 0);
    E(IQNUM, dst->asdcdef.iqnum);
  
    i = asdcalloc(dst, 4*L(IQNUM));
    E(IQPTR1, i);
    i = asdcalloc(dst, 4*L(IQNUM));
    E(IQPTR2 ,i);
    E(IQCNT1 ,0);
    E(IQCNT2 ,0);
  
    /* Initialisation de la table des protocoles :                     */
    /*  - 1553B, "Dynamic bus control reject", "real time mon." inhibe */
    /*  et validation de l'usage SA=31 pour commandes codees           */
    E(PROPTR, asdcalloc(dst, 64));
    for (i=L(PROPTR); i<L(PROPTR)+64; i++) E(i, 0);
  
    /* Initialisation de la table des mots d'etat : */
    /*  - tous les RT sont inhibes                  */
    E(SWTPTR, asdcalloc(dst, 128));
    for (i=L(SWTPTR); i<L(SWTPTR)+128; i++) E(i ,0);
  
    /* Initialisation de la table des adresses et des tempos: */
    /*  - tous les RT sont inhibes : pas de tampons prevus    */
    E(ATPTR, asdcalloc(dst, 128));
    for (i=L(ATPTR); i<L(ATPTR)+128; i++) E(i ,0);
  
    /* Initialisation de la table des filtres :           */
    /*  - tous les RT sont inhibes : table mise a zero    */
    E(FTPTR, asdcalloc(dst, 64));
    for (i=L(FTPTR); i<L(FTPTR)+64; i++) E(i ,0);
  
    /* Reservation de la table des "Transmit Last Command" :   */
    /* et initialisation a une valeur absurde (cmd impossible) */
    E(LCDPTR, asdcalloc(dst, 64));
    for (i=L(LCDPTR); i<L(LCDPTR)+64; i++) E(i ,0xFFFF);
  
    /* Reservation de la table des "Last Status Word" : */
    /* et initialisation a une valeur absurde           */
    E(LSWPTR, asdcalloc(dst, 64));
    for (i=L(LSWPTR); i<L(LSWPTR)+64; i++) E(i ,0xFFFF);
  
    /* Initialisation de la table des "Last Synchro Word" :  */
    /*  -> Initialisee par 0xF0F0 a des fins de surveillance */
    E(LSYPTR, asdcalloc(dst, 64));
    for (i=L(LSYPTR); i<L(LSYPTR)+64; i++) E(i, 0xF0F0);
  
    /* Initialisation de la table des Masques d'IT Commandes Codees : */
    E(MIMPTR, asdcalloc(dst, 128));
    for (i=L(MIMPTR); i<L(MIMPTR)+128; i++) E(i ,0);
  
    /* Initialisation de la table des "BITs" :  */
    E(BITPTR, asdcalloc(dst, 128));
    for (i=L(BITPTR); i<L(BITPTR)+128; i++) E(i ,0);
  
    /* Initialisation de la table des "Phases" :  */
    E(RTPPTR, asdcalloc(dst, 64));
    for (i=L(RTPPTR); i<L(RTPPTR)+64; i++) E(i ,0);
  
    /* Initialisation de la table des "Transmit Vector Word" :  */
    E(TVWPTR, asdcalloc(dst, 64));
    for (i=L(TVWPTR); i<L(TVWPTR)+64; i++) E(i ,0);
  
    /* Initialisation de la table des commandes codees "RESERVE" :  */
    // For M51 only
    // E(RESPTR, asdcalloc(dst, 32));
    // for (i=L(RESPTR); i<L(RESPTR)+32; i++) E(i ,0);
  
    /* Initialisation des pointeurs utilises en mode BC */
    E(BCIPTR, 0);   /* En esperant que ca suffira */
  
    /* Initialisation du mode moniteur :                    */
    E(MBLEN, dst->asdcdef.mbleng);
    k = asdcalloc(dst, (int) L(MBLEN));
    if (k==-1) { RETURN(ENOMEM);     /* Plus de memoire */
               }
    E(M1PTR, k);
    k = asdcalloc(dst, (int) L(MBLEN));
    if (k==-1) { RETURN(ENOMEM);     /* Plus de memoire */
               }
    E(M2PTR, k);
    E(MBFLG, 0);
  
    /* Initialisation de quelques parametres */
    /*    --> Ces initialisations sont sans doute inutiles, car */
    /*        ecrasees par les valeurs par defaut du firmware   */
    /*        a l'appel de ASDCGO.                              */
    /*    ---> Elles sont donc repetees juste apres ASDCGO      */
    E(BCSMSK, dst->asdcdef.bcsmsk); /* BC Status Word Mask         */
    E(BCIGP, dst->asdcdef.bcigp);   /* BC Inter message Gap Time   */
    E(BRTCNT, dst->asdcdef.brtcnt); /* Bc ReTry CouNT              */
    E(BRTBUS, dst->asdcdef.brtbus); /* Bc ReTry BUS                */
    E(RSPGPA, dst->asdcdef.rspgpa); /* Temps de non rep. RT        */
printk("Ecriture de %d dans rspgpa [0x%X]\n", RSPGPA, dst->asdcdef.rspgpa);
    E(RSPGPS, dst->asdcdef.rspgps); /* Temps de reponse RT simules */
printk("Ecriture de %d dans rspgps [0x%X]\n", RSPGPS, dst->asdcdef.rspgps);
  
    /* (Re)Initialisation de la table des commandes codees */
    for (i=0; i<2; i++)
       for (j=0; j<32; j++)
          for(k=0; k<(COCO_MAX+1); k++)
             dst->cocor[i][j][k] = (struct scoco) { 0, 0, 0, 0};
  
  
    /* Initialisation des tampons flux BC a "tous disponibles" */
#ifdef IMPLEMENTATION_DES_TRAMES_DU_MODE_GERANT
    dst->pbcf[0].s = NULL;
    for (j=1; j<dst->nombre_tampons_flux; j++)
       { dst->pbcf[j].s = &(dst->pbcf[j-1]);
       }
    dst->pbcfl = &(dst->pbcf[dst->nombre_tampons_flux - 1]);
    dst->nb_tamp_flux_dispos = dst->nombre_tampons_flux;
#endif

    /* Initialisation des pointeurs vers la chaine des flux */
    dst->pflux = dst->dflux = 0;
  
    /* Seule la premiere des 4 lignes ci-dessous est indispensable */
    dst->tf_attente = 0;
    dst->tf_zd = 0;
    dst->tf_flux = 0;
    dst->tf_pta = NULL;
  
    /* Initialisation variable de chainage des blocs dans une trame   */
    /* (utilisee pour memoriser "bloc precedent" : pas trop propre et */
    /*  non reentrant !!!)                                            */
    /// bloc_suivant = 0;
  
    /* Initialisation de la memoire "image" */
    for (i=0; i<65536; i++) EI(i, 0L, 302);



    /* Liberation de tous les tampons des FIFOs associees aux */
    /* voies en emission :                                    */
    for (i=0; i<2; i++)
        for (j=0; j<32; j++)
            for (k=0; k<32; k++)
                viderCompletementFifo(&(dst->pTampEcr[i][j][k]));

    /* Initialisation de la liste des "wait queues" Linux */
    wq_init(dst);
  
    /* Initialisation de l'indicateur "trame en cours" */
    dst->fin_bc = 1;
  
    /* Initialisation (au cas ou ...) du fanion "RAZ en cours" */
    dst->raz = 0;
  
    /* Initialisation histogramme d'utilisation de la table des ITs */
    dst->nbhistoit = 0;
    dst->deborde = 0;
    for (i=0; i<ASDCTHISTOIT; i++) dst->histoit[i] = 0;
  
    /* Initialisation des enchainements de trame */
    dst->idtrame = 0;
    for (i=0; i<MAXDESCRT; i++) dst->descr_trame[i].idtrame = 0;
  
    /* Pour debug : initialisation du tampon des messages */
    kkprintf("asdcraz : ph0\n");
    dst->msg.ch = dst->asdcmsg_ch;
    for (i=0; i<TTMSG; i++) dst->msg.ch[i] = '\0';
    dst->msg.ic = 0;
  
    /* Et premiere utilisation de ce tampon ... */
    MSG("Initabi (ASDCRAZ)\n");
    MSGFIN;

    /* Indicateur "initialisation effectuee au moins une fois" */
    dst->jamais_initialise = 0;
  
   RETURN(OK);



}


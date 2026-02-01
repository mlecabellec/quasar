
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDC_CC_GETDATA et ASDC_CC_SETDATA : Fonctions ioctl d'acces au mot
                                     de donnee d'une commande codee.

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
22/04/2013  YG   Version initiale
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/



#include "driverIncludes.h"



/// TODO : Seuls arguments des fonction ci-dessous qui sont sans doute
///        necessaires : dst et arg !!!


/* Lecture du mot de data associe a une commande codee :                    */
/* Le champ asdcvoie.nmots est utilise pour passer le mot de data           */
/* Le mot mode est utilise pour definir la cc concernee et, indirectement,  */
/* la data concernee.                                                       */
long
do_ASDC_CC_GETDATA(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    struct asdcvoie *v, zv;
    int j, k;

    // printk("asdcioctl.c entree dans case ASDC_RT_GETTREP\n");
    TDUTIL(arg, sizeof(struct asdcvoie));
    DUTIL(zv, v, arg, sizeof(struct asdcvoie));
    TVUTIL(zv, v, arg, sizeof(struct asdcvoie));


    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, pas de controle a proprement parler, */
    /*  on se contente de supprimer tous les bits inutilises    */
    v->bus &= 0x01;
    v->adresse &= 0x1F;


    switch(v->mode) {
        case  2 :        /* Transmit (last) status word */
            k = L(LSWPTR);
            /* La table est-elle dans la zone memoire autorisee ? */
            if ((k < 0X400) || (k >= (TAILLEMEM - 64))) RETURN(EADDRNOTAVAIL);
            j = k + v->adresse + 32 * v->bus;
        break;

        case 16 :        /* Transmit vector word */
            k = L(TVWPTR);
            /* La table est-elle dans la zone memoire autorisee ? */
            if ((k < 0X400) || (k >= (TAILLEMEM - 64))) RETURN(EADDRNOTAVAIL);
            j = k + v->adresse + 32 * v->bus;
        break;

        case 17 :        /* Synchronize with data word */
            k = L(LSYPTR);
            /* La table est-elle dans la zone memoire autorisee ? */
            if ((k < 0X400) || (k >= (TAILLEMEM - 64))) RETURN(EADDRNOTAVAIL);
            j = k + v->adresse + 32 * v->bus;
        break;

        case 18 :        /* Transmit last command word */
            k = L(LCDPTR);
            /* La table est-elle dans la zone memoire autorisee ? */
            if ((k < 0X400) || (k >= (TAILLEMEM - 64))) RETURN(EADDRNOTAVAIL);
            j = k + v->adresse + 32 * v->bus;
        break;

        case 19 :        /* Transmit BIT word */
            k = L(BITPTR);
            /* La table est-elle dans la zone memoire autorisee ? */
            if ((k < 0X400) || (k >= (TAILLEMEM - 128))) RETURN(EADDRNOTAVAIL);
            j = k + v->adresse + 64 * v->bus;
        break;

        case 20 : /* Selected ransmitter shutdown  : NON IMPLEMENTE */
        case 21 : /* Override selected ransmitter shutdown  : NON IMPLEMENTE */
        default : /* Unknown mode code or mode code without data word */
            /* Commande codee sans data, inexistante ou non implementee */
            RETURN(EDOM);
    }

    /* Lecture du mot de donnee */
    v->nmots = L(j);
    v->nmots &= 0xFFFF;       /* C'est un mot de 16 bits */

    /* Retour de la donnee a l'appli appelante */
    VUTIL(zv, v, arg, sizeof(struct asdcvoie));

    RETURN(OK);
}



/* Ecriture du mot de data associe a une commande codee :         */
/* Le champ asdcvoie.nmots est utilise pour passer le mot de data */
/* Le mot mode est utilise pour definir la data concernee         */
long
do_ASDC_CC_SETDATA(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    struct asdcvoie *v, zv;
    int j, k;

    // printk("asdcioctl.c entree dans case ASDC_RT_GETTREP\n");
    TDUTIL(arg, sizeof(struct asdcvoie));
    DUTIL(zv, v, arg, sizeof(struct asdcvoie));


    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, pas de controle a proprement parler, */
    /*  on se contente de supprimer tous les bits inutilises    */
    v->bus &= 0x01;
    v->adresse &= 0x1F;
    v->nmots &= 0xFFFF;       /* Mot de donnee : 16 bits */


    switch(v->mode) {
        case  2 :        /* Transmit (last) status word */
            k = L(LSWPTR);
            /* La table est-elle dans la zone memoire autorisee ? */
            if ((k < 0X400) || (k >= (TAILLEMEM - 64))) RETURN(EADDRNOTAVAIL);
            j = k + v->adresse + 32 * v->bus;
        break;

        case 16 :        /* Transmit vector word */
            k = L(TVWPTR);
            /* La table est-elle dans la zone memoire autorisee ? */
            if ((k < 0X400) || (k >= (TAILLEMEM - 64))) RETURN(EADDRNOTAVAIL);
            j = k + v->adresse + 32 * v->bus;
        break;

        case 17 :        /* Synchronize with data word */
            k = L(LSYPTR);
            /* La table est-elle dans la zone memoire autorisee ? */
            if ((k < 0X400) || (k >= (TAILLEMEM - 64))) RETURN(EADDRNOTAVAIL);
            j = k + v->adresse + 32 * v->bus;
        break;

        case 18 :        /* Transmit last command word */
            k = L(LCDPTR);
            /* La table est-elle dans la zone memoire autorisee ? */
            if ((k < 0X400) || (k >= (TAILLEMEM - 64))) RETURN(EADDRNOTAVAIL);
            j = k + v->adresse + 32 * v->bus;
        break;

        case 19 :        /* Transmit BIT word */
            k = L(BITPTR);
            /* La table est-elle dans la zone memoire autorisee ? */
            if ((k < 0X400) || (k >= (TAILLEMEM - 128))) RETURN(EADDRNOTAVAIL);
            j = k + v->adresse + 64 * v->bus;
        break;

        case 20 : /* Selected ransmitter shutdown  : NON IMPLEMENTE */
        case 21 : /* Override selected ransmitter shutdown  : NON IMPLEMENTE */
        default : /* Unknown mode code or mode code without data word */
            /* Commande codee sans data, inexistante ou non implementee */
            RETURN(EDOM);
    }


    /* Ecriture du mot de donnee                                    */
    E(j, v->nmots);

    RETURN(OK);
}


/************************************************************/
/* Fonctions de "log "                                      */
/*                                                          */
/* Cette fonction permet d'afficher des messages (info,     */
/* erreur, etc...) dans syslog en fonction du parametrage   */
/* choisi et en fonction de la nature du message.           */
/*                                                          */
/*                      ASTRIUM ST - TE641 - Yves Guillemot */
/************************************************************/

/*
Quand       Qui   Quoi
----------  ----  -------------------------------------------------------------
26/03/2010  yg    - Version initiale sous QNX (resource manager et applis)
 8/10/2013  yg    - Adapatation au noyau Linux

*/



#include "DriverIncludes.h"


/* Valeurs par defaut des seuils d'ecriture */
static int seuilSyslog = DRV_INFO;
static int seuilStderr = DRV_WARNING;



/*
 * Modification d'un seuil d'ecriture :
 *   - destination peut etre soit SYSLOG, soit STDERR
 *   - niveau est une valeur entre DRV_DEBUG et DRV_FATAL
 *
 * Le seuil d'ecriture associe a la sortir <destination> est
 * regle a la valeur <niveau>.
 */
void initPrintLog(int destination, int niveau)
{
printk("destination=%d niveau=%d SYSLOG=%d\n", destination, niveau, SYSLOG);
    if (destination == SYSLOG) {
      printk("%s: Nouveau loglevel = %d\n", PCIE6509_NOM, niveau);  
    }

    switch(destination) {
        case SYSLOG :
            seuilSyslog = niveau;
            break;
        case STDERR :
            seuilStderr = niveau;
            break;
        default :
            printLog(DRV_WARNING,
                     "initLog: destination %d anormale\n",
                     destination);
            break;
    }
}


/*
 * Ecriture d'un message d'information ou d'erreur :
 *   Le message est ecrit sur stderr si msglevel >= seuilStderr
 *   Le message est ecrit sur syslog si msglevel >= seuilSyslog
 *
 * Les seuils ont des valeurs par defaut definies au debut de ce fichier.
 * Ils peuvent être modifies a tout moment en utilisant la fonction initLog().
 *
 * Les valeurs de msglevel possibles sont :
 *    DRV_FATAL   : Erreur imposant un arret du driver
 *    DRV_ERROR   : Une erreur s'est produite, mais le driver pourrait continuer
 *                  a fonctionner
 *    DRV_WARNING : Un evenement anormal s'est produit
 *    DRV_INFO    : Une information
 *    DRV_DEBUG   : Message utile en phase de mise au point uniquement
 */
void printLog(const int msglevel,
              const char *format,
                      ...)
{
    va_list ap;
    char * loglevel;
    char formatComplet[150];
    char *fmt;
    
    fmt = formatComplet;

/* Le code commente ci-desous etait valide sous QNX, ou le "resource    */
/* manager" est une application en espace utilisateur. Il n'est plus    */
/* fonctionnel sous Linux ou le driver est execute dans l'espace noyau. */
/*
    if (msglevel >= seuilStderr) {
        va_start(ap, format);
            vfprintf(stderr, format, (va_list)ap);
        va_end(ap);
    }
*/

    if (strlen(format) > (sizeof(formatComplet) - 5)) {
        printk(KERN_CRIT "qabil:logMsg(): format trop long : \n\"%s\"\n",
               format);
        return;
    }
    
    if (msglevel >= seuilSyslog) {

        switch(msglevel) {
            case DRV_FATAL :   loglevel = KERN_CRIT;    break;
            case DRV_ERROR :   loglevel = KERN_ERR;     break;
            case DRV_WARNING : loglevel = KERN_WARNING; break;
            case DRV_INFO :    loglevel = KERN_INFO;    break;
            case DRV_DEBUG :   loglevel = KERN_DEBUG;   break;
            default :          loglevel = KERN_ERR; /* Ne devrait pas arriver */
        }

        strcpy(formatComplet, loglevel);
        strcat(formatComplet, format);

        va_start(ap, format);
            vprintk(format, ap);
        va_end(ap);
    }
}






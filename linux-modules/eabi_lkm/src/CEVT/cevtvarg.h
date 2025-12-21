/************************************************************************
 *                                                                      *
 *     Pseudo-driver "Concentrateur d'EVenemenTs" pour ASDC et ETOR     *
 *     ------------------------------------------------------------     *
 *                                                                      *
 *                     VARIABLES GLOBALES DU PILOTE                     *
 *                                                                      *
 *     ou, pour LynxOS, STRUCTURE DES VARIABLES STATIQUES DU PILOTE     *
 *                                                                      *
 *                                                                      *
 *                                         Y.Guillemot, le  9/09/2002   *
 *                           Implementation du "select" le 23/09/2002   *
 *	           Debut traitement compatibilite Linux le  3/10/2002   *
 *        Adapte a kernel recent (> 2.6.18) et EMUABI : le 18/04/2013   *
 ************************************************************************/



#if !defined( _CEVTVARG_H )
#define _CEVTVARG_H


 
struct cevt_statics
{
   int dioctl;		/* Debug : Dernier ioctl appele */
   int dphioctl;	/* Debug : Phase ioctl en cours */
   int dit;		/* Debug : Derniere IT appelee */
   int dphit;		/* Debug : Phase IT en cours */

   int numero;		/* Numero du device CEVT */
   
   /* Semaphore de protection des fonctions d'allocation des fluxs */
   int mutexflux;
   
   /* Queue et autres conditions d'attente d'un evenement */
   wait_queue_head_t sem_attente;
   spinlock_t lock_attente;
   int cond_attente;
      
   /* Nombre total de tampons pour flux RT_evt et pointeur vers ces tampons */
   int nb_tampons;
   struct cevt_tfch *pbcfev;		/* Pointe la base du tableau qui  */
					/* contient tous les tampons      */
   
   /* Nombre de tampons restants et pointeur vers le premier de ces tampons */
   int nb_tamp_dispos;
   struct cevt_tfch *ptv;		/* Pointe le premier element de la */
					/* chaine des tampons libres       */

   struct cevt_tfch *ppt;  /* Pointeur premier tampon plein */
   struct cevt_tfch *pdt;  /* Pointeur dernier tampon plein */
      
   int raz;		/* Indicateur de RAZ driver en cours (CEVT_AVORTE) */
   
   
   /* Variables necessaires a l'implementation de select() */
   int *rsel_sem; 	/* sem for select read */
   int sel_data; 	/* data available for read */
   // int *wsel_sem; /* sem for select write */		// non implemente
   // int *esel_sem; /* sem for select exception */	// non implemente
   // int n_spacefree; /* space available for write */	// non implemente
   // int error; /* error condition */			// non implemente
          
};






#endif /* !defined( _CEVTVARG_H ) */

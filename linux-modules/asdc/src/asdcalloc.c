/************************************************************************
 *                                                                      *
 *      Driver pour carte AMI d'interface 1553 (fabriquee par SBS)      *
 *     ------------------------------------------------------------     *
 *                                                                      *
 *        Fonctions d'allocation de blocs dans la memoire de l'AMI      *
 *                                                                      *
 *                                         Y.Guillemot, le 23/01/1991   *
 *                                      derniere modif. le 29/01/1991   *
 *                         Adaptation a microcode SDC : le 31/03/1992   *
 *            Modification pour gestion memoire libre : le 17/05/1994   *
 *                                    derniere modif. : le 21/10/1994   *
 *                                                                      *
 *                  Adaptation a LynxOS (v4.0) : modif. le  6/04/2001   *
 *           Adaptation au source commun LynxOS/Linux : le 30/09/2002   *
 *                                                                      *
 * Regroupement des 2 coupleurs d'une meme carte en un                  *
 *                                 seul device (v4.7) : le 18/10/20002  *
 *                                                                      *
 *                  Source unique LynxOS/Linux (v4.9) : le  8/01/2003   *
 *                                                                      *
 * Ajout inclusion de sched.h suite a la modif.                         *
 *                     du fichier interface.h (v4.11) : le 21/04/2004   *
 *                                                                      *
 *                  Adaptation a Linux v2.6.x (V4.12) : le 26/10/2004   *
 *                                                                      *
 *           Modif. mineure dans les #include (V4.16) : le  8/09/2005   *
 *                                                                      *
 *                           Adaptation � LynxOS v4.0 : le 18/09/2006   *
 *                                                                      *
 ************************************************************************/
 
 
 
 /*
    PRINCIPE DE GESTION DE LA MEMOIRE UTILISE :
        
           memo[] : table de structure qui decrit les blocs libres          
              memo.a = adresse debut bloc libre
              memo.l = longueur du bloc (-1 si entree de memo[] est inutilisee)
              memo.s = indice (dans table memo) du bloc suivant, -1 si fini
              memo.p = indice (dans table memo) du bloc precedent, -1 si fini
              
           premier : indice dans table memo[] du plus petit bloc libre
           dernier : indice dans table memo[] du plus grand bloc libre
           
           Les indices (.s et .p) des blocs sont organises de facon a
           permettre le balayage des blocs disponibles du plus petit au plus
           grand.
    
       
 */


/* Retirer commentaire ci-dessous pour debuguer l'allocation memoire */
/* (Attention : beaucoup, beaucoup de sorties sur la console !!!)    */
// #define DEBUG

/* Choisir ci-dessous la destination des messages de debug */
/* (LynxOS uniquement, pas de choix possibl sous Linux)    */
#ifdef LYNXOS
// # define PRINTF kkprintf	/* Sortie sur le terminal systeme (de boot) */
# define PRINTF cprintf	/* Sortie sur la console courante */
#elif LINUX
# define PRINTF printk
#else
# error	LINUX ou LYNXOS doit etre defini
#endif	/* LYNXOS ou LINUX */

/* REMARQUE : Suite a probleme constate le 5/6/2001, si utilisation debug */
/*            avec cprintf(), pour voir la fin des affichages dans la     */
/*            fenetre courante (newconsole) : mettre un sleep(1) avant    */
/*            l'instruction exit() du programme principal.                */


#ifdef LYNXOS

# ifdef LYNX_V4
#  include <types.h>
# endif /* LYNX_V4 */

# include <io.h>
# include <mem.h>
# include <sys/file.h>
# include <kernel.h>
# include <dldd.h>
# include <stdio.h>
# include <errno.h>

#elif LINUX

# include <linux/module.h>
# include <linux/kernel.h>
# include <linux/fs.h>
# include <linux/errno.h>
# include <linux/version.h>
# if LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0) 
# include <linux/sched/signal.h>
# include <linux/uaccess.h>
# else
# include <linux/sched.h>
# endif

#endif	/* LYNXOS ou LINUX */


 
#define KERNEL 

#include "interface.h"	/* Macros de portage LynxOS/Linux */
#include "asdcctl.h"	/* Structures de donnees et codes ioctl */
#include "asdcvarg.h"	/* Variables statiques */


/* Macros pour simplifier l'ecriture (portage de SUN-OS vers LynxOS) */
#define memo		driver_statics->memo
#define premier		driver_statics->ipremier
#define dernier		driver_statics->idernier
#define unit		driver_statics->signal_number
#define mutexalloc	driver_statics->mutexalloc



#ifdef DEBUG
# define dprintf PRINTF
  static void affmemo(struct asdc_statics *asdc_varg)
  {  int i;
     dprintf("\nAFFMEMO :   unit=%d   premier=%d   dernier=%d\n",
            unit, premier, dernier);
     for (i=0; i<TMEMO; i++)
        { if (memo[i].l != 0xFFFF)
            dprintf("   i=%d   a=0x%x   l=%d   s=%d   p=%d\n",
                   i, memo[i].a, memo[i].l,
                   memo[i].s, memo[i].p);
        }
     dprintf("\n");
  }
#else /* DEBUG */
# define affmemo
# define dprintf
#endif /* DEBUG */


/* Suppression d'une entree (d'indice i) dans la chaine memo[] */
static void oter(struct asdc_varg *driver_statics, int i)
{
   int j;
   
   dprintf("oter(%d, %d)\n", unit, i);
   
   j = memo[i].p;
   if (j != -1) memo[j].s = memo[i].s;
           else premier = memo[i].s;
        
   j = memo[i].s;
   if (j != -1) memo[j].p = memo[i].p;
           else dernier = memo[i].p;

   affmemo(driver_statics);
}


   
/* Introduction d'une nouvelle entree (d'indice i) dans la chaine memo[] */
/* L'insertion est effectuee APRES l'entree d'indice j                   */
static void inserer(struct asdc_varg *driver_statics, int i, int j)
{  int k;

   dprintf("inserer(%d, %d, %d)\n", unit, i, j);
    
   k = memo[j].s;
   
   memo[i].p = j;
   memo[i].s = k;
   
   memo[j].s = i;
   if (k==-1) dernier = i;
         else memo[k].p = i;
         
   affmemo(driver_statics);
}


   
/* Ajout d'une nouvelle entree (d'indice i) dans la chaine memo[] */
/* L'insertion est effectuee a l'endroit approprie                */
static void ajouter(struct asdc_varg *driver_statics, int i)
{  int j, k;

   dprintf("ajouter(%d, %d)\n", unit, i);
   
   j = premier;
   
   if (memo[j].l >= memo[i].l)  /* Insertion en tete de chaine */
      { memo[i].s = premier;
        memo[i].p = -1;
        memo[premier].p = i;
        premier = i;
        
        affmemo(driver_statics);
        
        SEMSIGNAL(&mutexalloc);		/* Fin section critique */
        return; 			/* Fin de l'operation */
      }
      
   for (;;)
      { k = memo[j].s;
        if (k==-1) break;          /* Insertion en fin de chaine */
        if (    (memo[j].l <= memo[i].l)
             && (memo[k].l >= memo[i].l)) break; 
            
        j = k;   /* Pour examen du bloc suivant */
      }
      
   inserer(driver_statics, i, j);   /* Execution de l'insertion */
}
   







/* Remise a zero du systeme d'allocation : toute la memoire est libre ! */
void asdcrazalloc(struct asdc_varg *driver_statics)
{ int i;

  /* La section critique est la pour eviter incoherences, mais il  */
  /* vaut mieux qu'aucune tache ne soit active en phase d'init ... */
  SEMWAIT(&mutexalloc, SEM_SIGIGNORE);	/* Debut section critique */

  dprintf("asdcrazalloc(%d)\n", unit);

  for (i=1; i<TMEMO; i++) memo[i].l = 0xFFFF;
  
  memo[0].a = DEBUT_RAM;		/* Base memoire "utilisateur"   */
  memo[0].l = FIN_RAM - DEBUT_RAM;	/* Taille memoire "utilisateur" */
  memo[0].s = -1;
  memo[0].p = -1;
     
  premier = 0;  
  dernier = 0; 
   
  affmemo(driver_statics);
  
  SEMSIGNAL(&mutexalloc);		/* Fin section critique */
}






/* Allocation d'une zone de n mots - asdcalloc() renvoie l'adresse de la zone */
/*   asdcalloc() renvoie -1 si la memoire est pleine ou trop fragmentee */
int asdcalloc(struct asdc_varg *driver_statics, unsigned int n)
{
   int r;
   int i, j;

   SEMWAIT(&mutexalloc, SEM_SIGIGNORE);	/* Debut section critique */
   
   dprintf("asdcalloc(%d, %d) ....... >\n", unit, n);

   /* Recherche dans memo[] du premier bloc assez grand ... */
   for (i=premier; i!=-1; i = memo[i].s)
      { 
        if (memo[i].l >= n) break;
      }
   if (i == -1)  
     { 
       dprintf("                    ....... > 0x%x\n", -1);
       affmemo(driver_statics);
       SEMSIGNAL(&mutexalloc);		/* Fin section critique */
       return -1; /* Memoire saturee ou trop fragmentee */
     }

   r = memo[i].a;		/* Adresse du bloc alloue */
   memo[i].l -= n;	/* Memoire restant dans bloc */
   memo[i].a += n;        /* Nouvelle adresse debut bloc libre */
   
   if (memo[i].l == 0) /* Alors, l'entree de memo[] n'est plus utilisee */
      { memo[i].l = 0xFFFF;  /* Signaler entree libre */
        oter(driver_statics, i);
        
        dprintf("                    ....... > 0x%x\n", r);
        affmemo(driver_statics);
        SEMSIGNAL(&mutexalloc);		/* Fin section critique */

        return r;	/* Fin de l'allocation */
      }   
   
   /* La taille de l'entree memo[] a diminue : faut-il trier memo[] ? */
   j = memo[i].p;
   if (j == -1) 
     { 
       dprintf("                    ....... > 0x%x\n", r);
       affmemo(driver_statics);
       SEMSIGNAL(&mutexalloc);		/* Fin section critique */

       return r;	/* Tri inutile : Fin allocation */
     }
   if (memo[j].l <= memo[i].l) 
     {
       dprintf("                    ....... > 0x%x\n", r);
       affmemo(driver_statics);
       SEMSIGNAL(&mutexalloc);		/* Fin section critique */

       return r;    /* Idem */
     }
   
   
   /* Il faut trier ! */
   oter(driver_statics, i);	/* Entree i retiree de la chaine */
   ajouter(driver_statics, i);	/* Entree i reinseree dans la chaine */
                                /* (au bon endroit)                  */
   
   dprintf("                    ....... > 0x%x\n", r);
   affmemo(driver_statics);
   SEMSIGNAL(&mutexalloc);		/* Fin section critique */

   return r;	/* Tri termine : Fin allocation */
}
   






/* Liberation d'une zone de n mots a l'adresse a           */
/*   asdclibere() renvoie -1 en cas d'incoherence, 0 sinon */
int asdclibere(struct asdc_varg *driver_statics,
               unsigned int a, unsigned int n)
{
   int i, j;
   int avant, apres;
   
   SEMWAIT(&mutexalloc, SEM_SIGIGNORE);	/* Debut section critique */

   dprintf("asdclibere(%d, 0x%x, %d)\n", unit, a, n);

    
   /*************************************************************************/
   /* ATTENTION : On ne fait aucun controle sur la coherence des arguments  */
   /*             a et n : ni avec la memoire de la carte (et ses adresses) */
   /*                      ni avec la memoire deja libre                    */
   /*                                                                       */
   /* De plus, la "liberation" d'un bloc deja libre est autorisee           */
   /*                                                                       */
   /* Par contre, la "liberation" d'un bloc dont une portion est libre et   */
   /* une autre allouee est interdite : creation possible d'incoherences    */
   /* dans le chainage des blocs                                            */
   /*                                                                       */
   /* Pour le moment, asdclibere() ne peut remarquer aucune des             */
   /* incoherences possibles !                                              */
   /*                                                                       */
   /*  ATTENTION : Les applications utilisatrices, elles, finiront toujours */
   /* par remarquer ces incoherences si elles existent !                    */
   /*************************************************************************/
     
   i = premier;
   
   if (i==-1) /* Toute la memoire est occupee ==> creation du premier bloc */
     { memo[0].a = a;	/* Base memoire "utilisateur" */
       memo[0].l = n;
       memo[0].s = -1;
       memo[0].p = -1;
       premier = 0;  
       dernier = 0;  
       affmemo(driver_statics);
       SEMSIGNAL(&mutexalloc);		/* Fin section critique */
       return 0; 
     }
   
   /* Recherche d'un bloc memoire libre adjacent au debut du bloc a liberer */
   for (i=premier, avant=-1; i!=-1; i = memo[i].s)
      { if ((memo[i].a + memo[i].l) == a) { avant = i;
                                            break;
                                          }
      }
   
   /* Recherche d'un bloc memoire libre adjacent a la fin du bloc a liberer */
   for (i=premier, apres=-1; i!=-1; i = memo[i].s)
      { if (memo[i].a == (a+n)) { apres = i;
                                  break;
                                }
      }
   
      
                       
    /*
        Maintenant, 4 cas sont possibles : 
           1 - bloc a liberer n'est adjacent a aucun bloc libre
           2 - bloc a liberer est adjacent a fin bloc libre
           3 - bloc a liberer est adjacent a debut bloc libre
           4 - bloc a liberer separe deux bloc libre
    */
    

    if ((avant==-1) && (apres==-1))
       { /* Cas 1 : bloc a liberer n'est adjacent a aucun bloc libre */
       
         dprintf("asdclibere : Bloc n'est adjacent a aucun bloc libre\n");
       
         /* Peut-etre le bloc est-il deja libre ? */
         for (i=premier; i!=-1; i = memo[i].s)
            { register struct smemo *m;
              m = &(memo[i]);
              if ((m->a < a) && ((m->a + m->l) > (a + n)))
                { affmemo(driver_statics);
                  SEMSIGNAL(&mutexalloc);	/* Fin section critique */
                  return 0;	/* Le bloc etait deja libre ! */
                                /* On renvoi OK, mais peut etre vaudrait-il */
                                /* mieux signaler Pb ...                    */
                }
            }
         
         /* Recherche d'une entree disponible dans memo[] */
         for (i=0; i<TMEMO; i++) if (memo[i].l == 0xFFFF) break;
             
         if (i==TMEMO) 
           { affmemo(driver_statics);
             SEMSIGNAL(&mutexalloc);		/* Fin section critique */
             return -1;   /* Memoire trop fragmentee : */
           }              /* Plus de place dans memo ! */

         /* Introduction des donnees */
         memo[i].a = a;
         memo[i].l = n;
         /* Introduction dans la chaine */
         ajouter(driver_statics, i);
         SEMSIGNAL(&mutexalloc);		/* Fin section critique */
         return 0;
       }
    

    if ((apres==-1))
       { /* Cas 2 : bloc a liberer est adjacent a fin bloc libre */

         dprintf("asdclibere : Bloc est adjacent a fin bloc libre\n");
                
         /* Augmentation taille du bloc libre */
         memo[avant].l += n;
   
         /* La taille de l'entree memo[] a augmente : faut-il trier memo[] ? */
        j = memo[avant].s;
        if (j == -1) 
          { affmemo(driver_statics);
            SEMSIGNAL(&mutexalloc);		/* Fin section critique */
            return 0;		/* Tri inutile : Fin liberation */
          }
        if (memo[j].l >= memo[avant].l) 
          { affmemo(driver_statics);
            SEMSIGNAL(&mutexalloc);		/* Fin section critique */
            return 0;   /* Idem */
          }
   
        /* Il faut trier ! */
        oter(driver_statics, avant);	/* Entree avant retiree de la chaine */
        ajouter(driver_statics, avant);	/* Reinsertion dans chaine */
                                        /* (au bon endroit)        */
        SEMSIGNAL(&mutexalloc);		/* Fin section critique */
        return 0;	/* Tri termine : Fin liberation */
       }
    

    if ((avant==-1))
       { /* Cas 3 : bloc a liberer est adjacent a debut bloc libre */
         
         /* Augmentation taille du bloc libre */
         memo[apres].a = a;
         memo[apres].l += n;
   
         /* La taille de l'entree memo[] a augmente : faut-il trier memo[] ? */
        j = memo[apres].s;
        if (j == -1) 
          { SEMSIGNAL(&mutexalloc);		/* Fin section critique */
            return 0;			/* Tri inutile : Fin liberation */
          }
        if (memo[j].l >= memo[apres].l) 
          { affmemo(driver_statics);
            SEMSIGNAL(&mutexalloc);		/* Fin section critique */
            return 0;   /* Idem */
          }
   
        /* Il faut trier ! */
        oter(driver_statics, apres);	/* Entree avant retiree de la chaine */
        ajouter(driver_statics, apres);	/* Reinsertion dans chaine */
                                        /* (au bon endroit)        */
        SEMSIGNAL(&mutexalloc);	/* Fin section critique */
        return 0;	/* Tri termine : Fin liberation */
       }
    

    /* Cas 4 : bloc a liberer separe deux blocs libres */
         
    dprintf("asdclibere : Bloc a liberer separe deux blocs libres\n");
         
    /* Augmentation taille du premier */
    memo[avant].l += n + memo[apres].l;
         
    /* Retrait du second bloc de memo[] */
    memo[apres].l = 0xFFFF;
    oter(driver_statics, apres);
   
    /* La taille d'une entree memo[] a augmente : faut-il trier memo[] ? */
    j = memo[avant].s;
    if (j == -1) 
      { affmemo(driver_statics);
        SEMSIGNAL(&mutexalloc);	/* Fin section critique */
        return 0;		/* Tri inutile : Fin liberation */
      }
    if (memo[j].l >= memo[avant].l) 
      { affmemo(driver_statics);
        SEMSIGNAL(&mutexalloc);	/* Fin section critique */
        return 0;   		/* Idem */
      }
   
    /* Il faut trier ! */
    oter(driver_statics, avant);	/* Entree avant retiree de la chaine */
    ajouter(driver_statics, avant);	/* Reinsertion dans chaine */
                                        /* (au bon endroit)        */
    SEMSIGNAL(&mutexalloc);	/* Fin section critique */
    return 0;		/* Tri termine : Fin liberation */
}
   







/**************************************************************/
/***   ATTENTION : Les 2 fonctions ci-dessous ne font pas   ***/
/***               forcement un bon usage de mutexalloc.    ***/
/***                                                        ***/
/***    La protection du semaphore devrait etre apportee    ***/
/***    a un niveau superieur (ioctl de debug ?) a celui    ***/
/***    de ces deux fonctions.                              ***/
/**************************************************************/



/*   Ces fonctions sont uniquement destinees au debug */
/*   de l'allocation en memoire d'echange             */
                        
/* Lecture de l'environnement table memo[]       */
void asdclecememo(struct asdc_varg *driver_statics, struct asdcememo *data)
{ SEMWAIT(&mutexalloc, SEM_SIGIGNORE);	/* Debut section critique */
    data->tmemo = TMEMO;
    data->ipremier = premier;
    data->idernier = dernier;
  SEMSIGNAL(&mutexalloc);		/* Fin section critique */
}                    



/* Lecture d'une entree de la table memo[]        */
int asdclecmemo(struct asdc_varg *driver_statics, struct asdcblibre *data)
{ int i;

  i = data->i;
  if ((i >= TMEMO) || (i < 0)) return EINVAL;		/* Indice anormal */
  
  SEMWAIT(&mutexalloc, SEM_SIGIGNORE);	/* Debut section critique */
    data->a = memo[i].a;
    data->l = memo[i].l;
    data->p = memo[i].p;
    data->s = memo[i].s;
  SEMSIGNAL(&mutexalloc);		/* Fin section critique */
  return 0;   /* Tout est OK */
}



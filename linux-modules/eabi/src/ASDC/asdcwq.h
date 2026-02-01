/*
** asdcwqlinux.h for Drivers Linux pour SBS1553 in /home/alex
**
** Made by BEAUGY Alexandre
** Login   <alex@epita.fr>
**
** Started on  Mon Oct 28 10:25:17 2002 BEAUGY Alexandre
** Last update Mon Nov 18 14:05:44 2002 BEAUGY Alexandre
**
** Prise en compte de la reecriture complete de asdcwq.c (suite
** a un mauvais fonctionnement manifeste)
**                                             Anonymized, le  8/09/2005
**
** Ajout compatibilite Linux 64 bits           Anonymized, le 12/01/2009
*/

#ifndef		ASDCWQLINUX_H_
# define	ASDCWQLINUX_H_


struct asdc_varg;


/* Structure pour definir les chaines de wait queues  */
/* utilisees sous Linux pour remplacer les semaphores */
/* LynxOS places directement en memoire image.        */
struct wq_liste
  {
    wait_queue_head_t wq;     /* Wait Queue */
    unsigned int      nbre;   /* Nombre d'attentes en cours */
    int               cond;   /* Condition autorisant le reveil */
    struct wq_liste   *p;     /* Precedent */
    struct wq_liste   *s;     /* Suivant */
  };

int wq_creer(struct asdc_varg *dst);
void wq_init(struct asdc_varg *dst);
void wq_liberer(struct asdc_varg *dst);

struct wq_liste * wq_prendre(struct asdc_varg *dst);
void wq_rendre(struct asdc_varg *dst, struct wq_liste * wq);
int wq_dormir(struct asdc_varg *dst, long *lynx_sem);
void wq_reveiller(struct asdc_varg *dst,  long *wq);

#endif /* ASDCWQLINUX_H_ */


/*

 DRIVER LINUX POUR EXPERIMENTATIONS...

asdcalloc.h : Prototypes des fonctions d'allocation en memoire d'echange

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 8/04/2013  YG   Version initiale.
13/06/2014  YG   Evolution du nom d'un fichier inclus.


*/



#ifndef ASDC_ALLOC_H_
#define ASDC_ALLOC_H_


#include "asdcwq.h"
#include "asdc.h"
#include "interfaces.h"


/* Prototypes des fonctions de gestion de la memoire du coupleur*/
extern void asdcrazalloc(struct asdc_varg *driver_statics);
extern int asdcalloc(struct asdc_varg *driver_statics, unsigned int n);
extern int asdclibere(struct asdc_varg *driver_statics,
                      unsigned int a, unsigned int n);
extern void asdclecememo(struct asdc_varg *driver_statics,
                         struct asdcememo *data);
extern int asdclecmemo(struct asdc_varg *driver_statics,
                       struct asdcblibre *data);



#endif   /* ASDC_ALLOC_H_ */

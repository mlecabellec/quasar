/*****************************************************************/
/* Ce fichier definit les symboles (et macros) permettant        */
/* d'adapter les sources a la machine et au compilateur utilises */
/*****************************************************************/

/* Definition du systeme utilise */
/* (si Turbo-C, __TURBOC__ est defini de facon automatique) */
/* #define PC_MSDOS */
#define SUN_BSD 
/* #define VAX_VMS */


/* Validation eventuelle du mode "debug" */
/* #define DEBUG */

/* Definition d'une macro pour debug */
#ifdef DEBUG
extern int mdc_debug;
#define TEST(n,f,v) if(mdc_debug<=(n)) fprintf(stderr, (f), (v));
#else
#define TEST(n,f,v)
#endif

/* Type du compilateur C */
#ifndef SUN_BSD
#define C_ANSI
#endif

/* Position de la declaration des fonctions de la bibliotheque "util" */
/*                - sur PC  : On suppose que la variable INCLUDE de   */
/*                            l'environnement donne acces a util.h    */
/*                - sur VAX : On suppose que le nom logique c_util    */
/*                            pointe sur le fichier convenable        */
/*                - sur SUN : On suppose que le fichier util.h se     */
/*                            trouve dans un repertoire TLPC relie    */
/*                            par un lien logique a /usr/include      */
#if defined(PC_MSDOS)
#define UTIL <util.h>
#elif defined(VAX_VMS)
#define UTIL "c_util"
#else  /* reste SUN_BSD */
#define UTIL <TLPC/util.h>
#endif

/* Pour permettre un controle de l'inclusion de config.h */
#define CONFIG

/* Selon les systemes, calloc() est definie dans malloc.h ou stdlib.h */
#ifndef VAX_VMS
#ifndef __TURBOC__
#define MALLOC
#endif
#endif 

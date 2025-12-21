/***************************************************************************/
/*	Fichier : convd_p.c						   */
/*      PROCEDURES DE CONVERSION DE CHAINE DE CRACTERES EN REEL            */
/*      DESTINE A ETRE APPELEE PAR UN PROGRAMME PASCAL		           */
/***************************************************************************/
/***************************************************************************/
/*     Clement      le : 03/06/1991                                        */
/*     Version      :    1.1                                               */
/***************************************************************************/

#include <strings.h>
#include "util.h"

struct ch80  {  int t;
		char c[80];
	     };

/***************************************************************************/
/***  conversion d'une chaine de caracteres en reel simple precision     ***/
/***************************************************************************/

int convchr(x,val)

struct ch80 *x;
float *val;

{
	int i;
	double r;
	if (x->t < 80) x->c[x->t] = '\0';
	i = conversiond(&x->c[0],&r); /* fonction de la bibliotheque C */
	*val = r;
	return i;
}

/***************************************************************************/
/***  conversion d'une chaine de caracteres en reel double precision     ***/
/***************************************************************************/

int convchrd(x,val)

struct ch80 *x;
double *val;

{
	int i;
	double r;
	if (x->t < 80) x->c[x->t] = '\0';
	i = conversiond(&x->c[0],&r); /* fonction de la bibliotheque C */
	*val = r;
	return i;
}


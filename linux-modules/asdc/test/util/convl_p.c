/***************************************************************************/
/*	Fichier : convd_l.c						   */
/*      PROCEDURES DE CONVERSION DE CHAINE DE CRACTERES EN ENTIER          */
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
/***  conversion d'une chaine de caracteres en entier 16 bits            ***/
/***************************************************************************/

int convchain(x,val)

struct ch80 *x;
short int *val;

{
	int i,r;
	if (x->t < 80) x->c[x->t] = '\0';
	i = conversionl(&x->c[0],&r); /* fonction de la bibliotheque C */
	*val = r;
	return i;
}

/***************************************************************************/
/***  conversion d'une chaine de caracteres en entier long 32 bits       ***/
/***************************************************************************/

int convchalin(x,val)

struct ch80 *x;
int *val;

{
	int i,r;
	if (x->t < 80) x->c[x->t] = '\0';
	i = conversionl(&x->c[0],&r); /* fonction de la bibliotheque C */
	*val = r;
	return i;
}

/***************************************************************************/

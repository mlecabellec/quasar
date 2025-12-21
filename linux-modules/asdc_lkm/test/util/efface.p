module efface;

/***************************************************************************/
/*	Fichier : efface.p						   */
/*      PROCEDURES D'EFFACEMENTDE L'ECRAN 				   */
/***************************************************************************/
/***************************************************************************/
/*     Quittard/Clement  le : 05/01/1993                                   */
/*     Version     	    : 1.1                                          */
/***************************************************************************/

#include "../util.i"

(* =====================================================================*)
(*	Procedure d'effacement de l'ecran a partir de ligne/colonne	*)
(* =====================================================================*)

procedure EFFACE_FECR(l,c : integer);
var
	i,lmax,cmax : integer;

function nombre_lignes : integer;  extern;
function nombre_colonnes : integer;  extern;

begin
	curseur_goto(l,c);
	write(' ');
	curseur_goto(l,c);
	terminal(EFFACE_FLI);
	lmax := nombre_lignes - 1;
	cmax := nombre_colonnes - 1;
	for i := l + 1 to lmax do
	begin
		 curseur_goto(i,0); write(' ');
  		 curseur_goto(i,0); terminal(EFFACE_FLI);
	end;
end;

(* =====================================================================*)
(*	Procedure d'effacement d'une ligne				*)
(* =====================================================================*)

procedure EFFACE_LIGN(l : integer);

begin
	curseur_goto(l,0); write(' ');
	curseur_goto(l,0);   terminal(EFFACE_FLI);
end;


(* =========================================================================*)

#if !defined( _ASDCINFO_H )
#define _ASDCINFO_H
/************************************************************************
 * File             : asdcinfo.h
 * Operating System : LynxOS/PowerPC V3.1
 *
 * History
 *
 * Edition Date     Comment                                        By
 * ------- -------- ---------------------------------------------- ---
 *       1  2/04/01 created                                        yg
 *       2 23/10/01 Ajout des flux BC                              yg
 *       3  4/06/02 Ajout des flux evenements RT                   yg
 *
 *       4 18/10/02 - Regroupement params de 2 coupleurs (v4.7)
 *                  - Suppression des entrees inutilisees          yg
 */



struct abi_pmc_info
{
	int   bivoie;			/* 0 si coupleur simple */
					/* 1 si coupleur double */
	
	int   signal_number_1;		/* Params coupleur 1 */	
	int   nombre_tampons_flux_1;
	int   nombre_wq_1;

	int   signal_number_2;		/* Params coupleur 2 */	
	int   nombre_tampons_flux_2;
	int   nombre_wq_2;
};



#endif /* !defined( _ASDCINFO_H ) */

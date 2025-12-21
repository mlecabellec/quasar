/************************************************************************
 * Fichier : cevtinfo.h
 * Systeme : LynxOS/PowerPC V3.1
 *
 * Role    :Definition de la structure info du pseudo-driver CEVT
 *
 * Historique
 *
 * Edition Date     Commentaire                                    Auteur
 * ------- -------- ---------------------------------------------- ---
 *       1 11/09/02 Creation                                       yg
 */


#if !defined( _CEVTINFO_H )
#define _CEVTINFO_H


struct cevtinfo
{
  long numero;		/* Numero du CEVT */
  long nb_tamp;		/* Nombre de tampons a allouer */
};



#endif /* !defined( _CEVTINFO_H ) */

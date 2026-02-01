/*******************************************************************/
/*   MONABI : Logiciel d'espionnage 1553 utilisant une carte ABI   */
/*                                                                 */
/*      Affichage "en clair" du fichier brut fourni par MONABI     */
/*      ------------------------------------------------------     */
/*                                                                 */
/*          Lecture et decodage d'un bloc DEBUT ou FIN             */
/*                                                                 */
/*                                   Y. Guillemot, le 16/12/1993   */
/*                                         modifie le  3/01/1994   */
/*                                         modifie le  6/01/1994   */
/*                                         modifie le  7/01/1994   */
/*                                         modifie le 13/01/1994   */
/*******************************************************************/


#include <stdio.h>

long int octet_courant;

static char *nom_jour[] = { "dimanche", "lundi", "mardi", "mercredi",
                            "jeudi", "vendredi", "samedi", "dimanche" 
                          };
                          
static char *nom_mois[] = { "janvier", "fevrier", "mars", "avril", "mai",
                            "juin", "juillet", "aout", "septembre",
                            "octobre","novembre", "decembre" 
                          };



void examheure()
{  
       int annee, mois, jour_mois, jour_semaine, heure, minute, seconde;
       int taille_bloc;
       int i;
       
       taille_bloc = lec_mot(1);
       
       if (taille_bloc != 8)
         { printf("   Taille bloc = %d  ==> Anormal !!!    (octet # 0x%lx) \n", 
                   taille_bloc, octet_courant);
           for (i=1; i<taille_bloc; i++) lec_mot(1);
           return;
         }
   
       annee = lec_mot(1);
       mois = lec_mot(1);
       jour_mois = lec_mot(1);
       jour_semaine = lec_mot(1);
       heure = lec_mot(1);
       minute = lec_mot(1);
       seconde = lec_mot(1);
       
       printf("%s %d %s %d   -   ",
               nom_jour[jour_semaine], jour_mois, nom_mois[mois], annee);
               
       printf("%02dh %02d' %02d\"\n", heure, minute, seconde);
          
}   

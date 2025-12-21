/*
   Pseudo-driver CEVT (Concentrateur d'EVenemenTs)
   ===============================================


V1.1 : Version initiale, le 7 octobre 2002
       - Implementation du "select" en lecture (ioctl CEVT_LIRE)
       - Inhibition des ITs pendant CEVT_signaler()
       - Fonctions de debug CEVT_LETAT et CEVT_LTAMPON
       - Correction bug dans CEVT_LIRE(mode=CEVT_RAZ)
       - Ajout du mode CEVT_RAZRAZ pour CEVT_LIRE
       - Suppression affichage console dans CEVT_signaler()

V1.2 : 20 janvier 2003
       - Ajout du traitement des evenements "timers VG4"

V. non referencee (!!) : debut 2003
       - Ajout du traitement des evenements E/S TOR

V1.3 : 20 juillet 2004
       - Mise en coherence des 2 dernieres versions
       - Ajout traitement des flux BC (pour I/F Perl)

V1.4 : 26 octobre 2006
       - Fin du portage (avec compatibilite) Linux
       - Adaptation a datation via TSC (Linux)

v1.5 : 9 octobre 2007
        - Changement des macros Linux de passage des parametres au module pour
          rester compatible des noyaux Linux > 6.16

v1.6 : 13 janvier 2009
        - Mise en compatibilite avec les systemes 32 et 64 bits

v1.7 : 1er decembre 2009
        - Adaptation au noyau Linux v2.6.31

v1.8 : 30 novembre 2010
        - Ajout de la fonction time-out en lecture du CEVT sous Linux

v1.9 : 8 avril 2013
        - Adaptation au kernel 3.3.6 : remplacement ioctl() par unlocked_ioctl()

v1.11 : 22 sept 2015
        - Ajout spinlock

*/




/* ATTENTION : Il est imperatif que chacune des 3 chaines definies    */
/*             ci-dessous fasse au plus 100 caracteres (sous peine de */
/*             debordement des structures prevues pour leur relecture */
/*             par une application).                                  */



#define CEVT_NOM		"CEVT"
#define CEVT_VERSION		"1.11"
#define CEVT_DATE		"22 sept 2015"





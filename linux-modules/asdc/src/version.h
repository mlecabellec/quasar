/*
   Pilote des cartes SBS
   =====================


V1.3 : Version initiale
       - Destinee a carte ABI-V + microcode V1.x
       - Fonctionnalites de simulation d'abonnes uniquement


V1.4 : - Destinee a carte ABI-V + microcode V2.x
                  ou carte ABI-V3 + microcode d'origine
       - Fonctionnalites de simulation d'abonnes (carte ABI-V)
         et d'espion sequentiel (carte ABI-V et ABI-V3)

V2.1 : Version inachevee
       - Destinee a carte ABI-V + microcode V3.x
                  ou carte ABI-V3 + microcode d'origine
       - Fonctionnalites de simulation d'abonnes (carte ABI-V),
         d'espion sequentiel (carte ABI-V et ABI-V3)
         et de gerant (inachevee) (carte ABI-V seulement)

V3.1 : - Destinee a carte ABI-V + microcode V2.x (ou V3.x ...)
                  ou carte ABI-V3 + microcode V4.x
       - Fonctionnalites de simulation d'abonnes (carte ABI-V),
         d'espion sequentiel (carte ABI-V et ABI-V3)
         et de gerant (carte ABI-V3 seulement)
       - Tailles des tables LSWPTR et TVWPTR augmentees a 64 mots
         (ABIV3-V4.1)

V3.2 : - Ajout fonction de programmation periodes Majeur/mineur
       - Ajout fonctions de relecture variables internes du pilote (pour
         debug)
       - Ajout fonction de lecture numero de version pilote et microcode ABI
       - Ajout fonction de lecture horloge de datation
       - Correction de divers bugs ...

v3.3 : - Correction d'un bug (IT sur voie RT simule : bus non pris en compte !)
       - Ajout (mais inhibees par des "#ifdef ASDC_TRAITEMENT_RT") du
         mecanisme
         permettant de deporter le traitement associe a un equipement simule
         dans la fonction d'IT

v3.4 : - Possibilite de programmer une adresse RT en mode "Espion temps reel"
	 en utilisant le bit de plus fort poids du champ emission/reception
	 des commandes ioctl() ASDCDEF, ASDCINH et ASDCVAL
       - Possibilite de lire avec ioctl(ASDCLEC) et ioctl(ASDCLECIT) un
         tampon
         d'une voie en EMISSION (seulement si cette voie appartient a une
         adresse programmee pour le mode espion temps-reel)
       - Possibilite d'allouer (et de liberer) des zones en memoire d'echange
         au moyen des 2 nouvelles fonctions ioctl() ASDCALLOUE et ASDCLIBERE
       - Possibilite d'echanger la chaine de tampons connectee a une voie
         avec une autre chaine de tampons au moyen d'une nouvelle fonction
         ioctl() ASDCXTRT
       - Ajout nouvelles fonctions ioctl() pour attendre une commande codee :
         ASDCLECMIM, ASDCECRMIM, ASDCLECCC et ASDCLECCCIT

       - Ajout de differentes fonctions pour le mode particulier
         "TRAITEMENT_RT"

v3.5 : - Ajout nouvelles fonctions ioctl() pour simuler abonnes en EMISSION :
	 ASDCITE et ASDCITERAZ
       - Possibilite de limiter taille des tampons RT alloues par ASDCDEF au
         nombre de mots utilise (en specifiant 0xFFFF comme "adresse tampon")


v4.0 : - Prototype d'une version pour ABI/ASF-PMC et LynxOS

v4.1 : - Version ABI-PMC2 et LynxOS livree sur PPIL
		- fonctions BC completes (y-compris les flux BC)
		- fonctions RT limitees aux voies asynchrones (version
		  provisoire) et a une implementation statique (sans
		  traitement des IT) des voies synchrones

v4.2 : - Version ABI-PMC2 et LynxOS livree sur SIVOL
		- fonctions RT completes (y-compris les flux d'evenements)

v4.3 : - Correction bug nbre sur des donnees retourne par ASDCLEC

v4.4 : - Ajout variables pour autopsie via SKBD
       - Correction BUG lie a valeurs <0 retournees par asdc_lire()

v4.5 : - Correction bug dans ASDCLEC (retour de donnees non disponible
         sur une voie synchrone en mode SDC_ATTENDRE)
       - Ajout de la fonction ioctl ASDCLVER
       - Ajout d'un mode "statique" pour les voies abonnes
       - Ajout de la remontee des adresses logiques et physique de la carte
         par ASDCLVER (pour DEBUG)
       - Correction de l'initialisation BCIGP, RSPGPA, etc...
       - Correction bug du a l'extension du signe de bloc_suivant
       - Ajout d'une detection des debordements de l'acces a la memoire
         d'echange

v4.6 : - Suppression du traitement local des "flux d'evenements RT" et
         utilisation, a la place, du pseudo-driver CEVT (Concentrateur
         d'EVenemenTs)
       - Suppression du code des restes specifiques du CMBA
       - Passage en "unsigned" de differentes variables entieres utilisees
         pour stocker des adresses en memoire d'echange (pb si > 0x8000)
       - Correction d'une enorme erreur d'adresse de stockage des erreurs
         de flux BC (fonction bcflux_err() dans asdc.c)
       - Debut modification du code pour compatibilite Linux
       - Correction bug dans ASDCDEF (melange de traitement entre les
         modes SYNC/STAT et le mode ASYNC
       - Ajout des fonctions CC en mode abonne (via connexion CEVT)

v4.7 : - Contournement du probleme des ITs sur les coupleurs doubles
	 en regroupant les donnees associees aux deux coupleurs d'une
	 meme carte dans un device unique (la fonction d'IT s'execute
	 alors toujours dans le meme contexte "device" quelle que soit
	 la ligne PCI utilisee (la bonne ou l'autre ...)).
	 Les 2 coupleurs qui occupent la meme carte seront differencies
	 par leur numero de mineur (0 et 1).

v4.8 : - Ajout fonction pour attendre l'execution d'un bloc BC particulier
	 (pour "Gerant Generique").

v4.9 : - Code C commun pour LynxOS et Linux
       - Correction du probleme interdisant de charger le firmware en flash
         avec les versions 4.7 et 4.8 (ajout fonction ioctl ASDCVFLASH)
       - Correction mauvaise initialisation de RSPGPA (doit etre faite
         avant l'activation du traitement des I/O)
       - Ajout de la fonction ioctl ASDCMBBC
       - Ajout de la fonction ioctl ASDC_ABO_MF
       - Ajout lecture horloges "IRIG" et "non IRIG" simultanement (mais,
         bien sur, une seule des deux est valide)
       - Ajout fonctions ioctl ASDC_ABO_IV et ASDC_ABO_VV

v4.10 : - Correction bug dans ASDCECR (mode SDC_ATTENDRE non fonctionnel)

v4.11 : - Correction bug macro SWAIT sous Linux (EINTR non remont� quand
          interception d'un signal pendant SWAIT)

v4.12 : - Ajout de la simulation d'abonnes en mode "Synchrone 2"
          (transmission uniquement)
        - Ajout fonctions d'enchainement des trames
        - Correction bug dans chainage inverse des tampons abonnes
        - Adaptation au noyau Linux v2.6
        - Ajout fonction ASDCHCALIBR (comparaison des horloges)

v4.13 : - Ajout fonction de debug/investigation firmware ASDCTRACE (10/3/2005)
        - Ajout simulation abonne "Synchrone 2" avec utilisation du nouveau
          firmware specifique
        - Ajout fonction ASDC_ABO_LIBERER

v4.14 : (31/5/2005)
        - Ajout fonction ASDCEECRF pour simulation abonne "Synchrone 2"
        - Suppression de la d�claration de license "GPL" (pour Linux)

v4.15 : (11/8/2005)
        - Correction bug dans ASDCEVT_ABO_AJOUTER (interdisait connexion
          d'une voie synchrone 2 a un CEVT)
        - Correction bug dans ASDCEECRF (cas ou les chaines 1 et 2
          sont de la meme taille)
        - Correction bug dans ASDCEVT_ABO_SUPPRIMER (interdisait deconnexion
          d'une voie synchrone 2 a un CEVT si tous les tampons sont vides)

v4.16 : (16/9/2005)
        - Correction fonctionnement des wait queues sous Linux (attente sur
          lect. ou ecr. abonne ne fonctionnait qu'un nombre limite de fois)
        - Generation erreurs de parite transitoires (via ASDC_ABO_MF)
        - Correction (interversions) codes DSP et Firmware dans ASDCLVER
        - Correction bug passage arg. ioctl(ASDCLECMEMO) sous Linux
          (entrainait un plantage de l'utilitaire visualloc sous Linux)
        - ASDC_ABO_LIBERER ne detruit plus la table des filtres (utilisee
          par l'espion sequentiel)

v4.17 : (25/10/2006)
        - Adaptation a LynxOS v4.0
        - Sous Linux, datation des tampons CEVT par le TSC

v4.18 : (8/6/2007)
        - Correction bug ASDC_ABO_VV : remontait (a tort) EADDRNOTAVAIL quand
          appel sur une voie en mode SYNC2 dont la chaine 1 etait vide.

v4.19 : (9/10/2007)
        - Changement des macros Linux de passage des parametres au module pour
          rester compatible des noyaux Linux > 6.16

v4.20 : (15/02/2008)
        - Correction de l'appel des macros Linux de passage des parametres
          au module.
        - Ajout inclusion de sched.h a asdcwq.h pour permettre compilation
          dans le noyau Linux v2.6.22.
        - Creation d'une Makefile "autonome" (le script "faire" n'est plus
          necessaire) pour Linux v2.6
        - Ajout fonction ioctl ASDC_ABO_MFX (generation d'erreurs de parite
          transitoires multiples en mode abonne).
        - Correction du "bug SIVOL-Q" sur les erreurs de parite transitoire.

v4.21 : (12/01/2009)
        - Mise en compatibilite avec les systemes 64 bits.

v4.22 : (1/12/2009)
        - Modifs mineures pour adaptation au noyau Linux v2.6.31

v4.23 : (8§04/2013)
        - Modifs mineures pour adaptation au noyau Linux v3.3.6
*/




/* ATTENTION : Il est imperatif que chacune des 3 chaines definies    */
/*             ci-dessous fasse au plus 100 caracteres (sous peine de */
/*             debordement des structures prevues pour leur relecture */
/*             par une application).                                  */

#define ASDC_NOM		"ASDC"
#define ASDC_VERSION		"4.24"
#define ASDC_DATE		"8 avril 2013"




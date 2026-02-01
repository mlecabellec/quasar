/*
   Pilote de la carte VFX70
   ========================
   
   
V1.1 : 19/04/2013
       - Version initiale 
       - Destinee initialement a l'emulation EMUABI
       - Supporte la connexion d'un driver d'application une fois le FPGA
         configure

V1.2 préliminaire : 18/06/2014
       - Suppression d'un message de debug oublie

V1.2 : 16/07/2014
       - Suppression des includes <asm/io.h>, <asm/system.h> et <asm/uaccess.h>
         qui sont inutiles avec les versions actuelles du noyau et interdisent
         la compilation sur certaines d'entre elles (dont RedHawk).
         
v2.1 : 26/02/2015
       - Version "monobloc" : les 3 drivers VFX70, ASDC etr CEVT sont compiles
         en un seul module emuabi.ko
*/

/* ATTENTION : Il est imperatif que chacune des 2 chaines definies    */
/*             ci-dessous fasse au plus 100 caracteres (sous peine de */
/*             debordement des structures prevues pour leur relecture */
/*             par une application).                                  */

#define VFX_NOM         "VFX70"
#define VFX_DATE        "26 fevrier 2015"
#define VFX_VERSION     2
#define VFX_REVISION    1



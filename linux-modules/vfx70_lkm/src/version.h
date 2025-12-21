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
*/

/* ATTENTION : Il est imperatif que chacune des 2 chaines definies    */
/*             ci-dessous fasse au plus 100 caracteres (sous peine de */
/*             debordement des structures prevues pour leur relecture */
/*             par une application).                                  */

#define VFX_NOM         "VFX70"
#define VFX_DATE        "16 juillet 2014"
#define VFX_VERSION     1
#define VFX_REVISION    2



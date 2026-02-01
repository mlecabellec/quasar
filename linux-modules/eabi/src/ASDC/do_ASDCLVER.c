
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCLVER : Fonction ioctl de lecture des numeros de version/revision
           des differents elements logiciels

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
19/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/



#include "driverIncludes.h"



/// TODO : Seuls arguments sans doute necessaires : dst et arg !!!

long
do_ASDCLVER(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    /* Lecture des numeros de version du pilote et du */
    /* firmware de la carte ABI.                      */
    /*    ATTENTION : La version du firmware n'est    */
    /*                pas significative si le dit     */
    /*                firmware n'est pas initialise   */
    /*                et en cours de fonctionnement.  */

    struct asdcver *ve, zve;
    char *p, *q;

    TVUTIL(zve, ve, arg, sizeof(struct asdcver));

    /* Copie du nom du pilote */
    p = ASDC_NOM;
    q = ve->nom;
    while (*p) { *q++ = *p++; }
    *q = '\0';

    /* Mise en forme et copie de la version du pilote */
    p = __stringify(ASDC_VERSION) "." __stringify(ASDC_REVISION);
    q = ve->version;
    while (*p) { *q++ = *p++; }
    *q = '\0';

    /* Copie de la date du pilote */
    p = ASDC_DATE;
    q = ve->date;
    while (*p) { *q++ = *p++; }
    *q = '\0';

    /* Copie des version et revision sous forme numerique */
    ve->ver = ASDC_VERSION;
    ve->rev = ASDC_REVISION;


    /* Copie des type, version et revision de la configuration du FPGA */
    ve->tfpga = (asdc_lecture_brute(dst->pcibar2, BAR2_MAGIC1) >> 16) & 0xFFFF;
    ve->vfpga = (asdc_lecture_brute(dst->pcibar2, BAR2_MAGIC1) >> 8) & 0xFF;
    ve->rfpga = asdc_lecture_brute(dst->pcibar2, BAR2_MAGIC1) & 0xFF;

    /* Si type est OK, copie des type, version et revision du firmware */
    /* (valide seulement si firmware en cours de fonctionnement) */
    ve->tfw = (asdc_lecture_brute(dst->pcibar2, BAR2_MAGIC2) >> 16) & 0xFFFF;
    ve->vfw = (asdc_lecture_brute(dst->pcibar2, BAR2_MAGIC2) >> 8) & 0xFF;
    ve->rfw = asdc_lecture_brute(dst->pcibar2, BAR2_MAGIC2) & 0xFF;


    /* Verifier que le tiroir interface est connecte et sous-tension */
    /* Recopier l'information                                        */
    ve->tiroir_ok = asdc_lecture_brute(dst->pcibar2, BAR2_CSR) & 0x80000000;

    /* Si tiroir present, copie de son type et de son numero de serie */
    ve->tiroir_type = (asdc_lecture_brute(dst->pcibar2, BAR2_UC_FPGA3) >> 8) & 0xFF;
    ve->tiroir_sn = asdc_lecture_brute(dst->pcibar2, BAR2_UC_FPGA3) & 0xFF;

    /* Si tiroir est present, copie des version et revision du soft */
    /* du microcontroleur principal.                                */
    ve->tiroir_vuc = (asdc_lecture_brute(dst->pcibar2, BAR2_UC_FPGA3) >> 24) & 0xFF;
    ve->tiroir_ruc = (asdc_lecture_brute(dst->pcibar2, BAR2_UC_FPGA3) >> 16) & 0xFF;


//     /* Copie des versions et revisions du DSP et du firmware     */
//     /* (valide seulement si firmware en cours de fonctionnement) */
//     ve->vdsp = L(PCODE);
//     ve->rdsp = L(VCODE);
//     ve->vfirm = L(FWPROC);
//     ve->rfirm = L(FWVERC);

//     /* Copie des adresses et du numero du coupleur */
//     ve->bvba = (long) dst->bvba;
//     ve->bpba = (long) dst->bpba;
//     ve->signal_number = (int) dst->signal_number;

    /* Envoi a l'application de la structure renseignee */
    VUTIL(zve, ve, arg, sizeof(struct asdcver));

    RETURN(OK);

}


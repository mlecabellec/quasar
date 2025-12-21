
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

asdcioctl.c : La fonction unlocked_ioctl()

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
23/02/2013  YG   Version initiale.


???????     YG   - Remplacement de ioctl() par unlocked_ioctl() a la suite de la
                   suppression du Big Kernel Lock du noyau Linux.
                 - Remplacement de l'appel de __ioremap() par celui de
                   ioremap_nocache().

23/02/2013  YG   Debut d'adaptation au driver ASDC de la carte EMUABI place
                 au dessus du driver de la carte VFX70.

 8/04/2013  YG   Deplacement de unlocked_ioctl() depuis asdc.c vers le
                 present fichier specifique.

11/06/2014  YG   Ajout des fonctions ASDC_SA_LIER et ASDC_SA_DELIER.
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/


#ifndef BUILDING_FOR_KERNEL
#define BUILDING_FOR_KERNEL	
#endif


#include "driverIncludes.h"



long
asdc_ioctl(int vfxNum, int asdcNum, struct file *fp,
               unsigned int cmd, unsigned long arg)
{

    struct asdc_varg *statics;
    statics = asdc_p[asdcNum];

    // kkprintf("Ioctl : Commande = 0x%0X\n", commande);
    statics->dioctl = cmd;
    statics->dphioctl = 1;

    // printk("ioctl : minor=%d cmd=%d arg=0x%lX\n", num, cmd, arg);
    // printk("        @statics=0x%p @mem=0x%p\n", statics, drv_p[num]->mem);

    // printk("ASDC : ioctl(0x%X)\n", cmd);

    switch( cmd )
    {


//         case GET_NAME_SIZE :
//         {
//             if (!access_ok(VERIFY_WRITE, (void __user *)arg, sizeof(int))) {
//                 return -EFAULT;
//             }
//             put_user(strlen(DRIVER_NOM), (int *)arg);
//             return 0;   /* OK */
//         }
//
        case SLV_GET_NAME :
        {
            char *nom = DEVICE_NAME;

            if(copy_to_user((void __user *)arg, nom, sizeof(DEVICE_NAME))) {
                return -EFAULT;
            }
            return 0;   /* OK */
        }

        case SLV_LEC_MEM :
        {
            Yg_DoubleInt di;
            
          //  printk("lecmem1\n");
            
            if (copy_from_user(&di, (void __user *)arg, sizeof(Yg_DoubleInt))) {
                return -EFAULT;
            }
            
            /// di.data2 = yg_read_mem(num, di.data1);
            
          //  printk("v=%d [0x%0X]\n", di.data2, di.data2);

            if (copy_to_user((void __user *)arg, &di, sizeof(di))) {
                return -EFAULT;
            }
            //  printk("lecmem4\n");
            return 0;   /* OK */
        }

        case sbs1553_read :
            return do_sbs1553_read(vfxNum, asdcNum, statics, arg);

        case sbs1553_write :
            return do_sbs1553_write(vfxNum, asdcNum, statics, arg);

        case ASDC_IMAGE_LIRE :
            return do_ASDC_IMAGE_LIRE(vfxNum, asdcNum, statics, arg);

        case ASDC_IMAGE_ECRIRE :
            return do_ASDC_IMAGE_ECRIRE(vfxNum, asdcNum, statics, arg);

        case ASDCACCES :
            return do_ASDCACCES(vfxNum, asdcNum, statics, arg);

        case ASDCAVORTE :
            return do_ASDCAVORTE(vfxNum, asdcNum, statics, arg);

        case ASDCRAZ :
            return do_ASDCRAZ(vfxNum, asdcNum, statics, arg);

        case ASDCEPAR :
            return do_ASDCEPAR(vfxNum, asdcNum, statics, arg);

        case ASDCLPAR :
            return do_ASDCLPAR(vfxNum, asdcNum, statics, arg);

        case ASDCGO :
            return do_ASDCGO(vfxNum, asdcNum, statics, arg);

        case ASDCSTOP :
            return do_ASDCSTOP(vfxNum, asdcNum, statics, arg);

        case ASDCDEF :
            return do_ASDCDEF(vfxNum, asdcNum, statics, arg);

        case ASDCECR :
            return do_ASDCECR(vfxNum, asdcNum, statics, arg);

        case ASDCLEC :
            return do_ASDCLEC(vfxNum, asdcNum, statics, arg);

        case ASDCMODE :
            return do_ASDCMODE(vfxNum, asdcNum, statics, arg);

        case ASDCLIREMODE :
            return do_ASDCLIREMODE(vfxNum, asdcNum, statics, arg);

        case ASDCLECEMEMO :
            return do_ASDCLECEMEMO(vfxNum, asdcNum, statics, arg);

        case ASDCLECMEMO :
            return do_ASDCLECMEMO(vfxNum, asdcNum, statics, arg);

        case ASDCALLOUE :
            return do_ASDCALLOUE(vfxNum, asdcNum, statics, arg);

        case ASDCLIBERE :
            return do_ASDCLIBERE(vfxNum, asdcNum, statics, arg);

        case ASDCEVT_ABO_AJOUTER :
            return do_ASDCEVT_ABO_AJOUTER(vfxNum, asdcNum, statics, arg);

        case ASDCEVT_ABO_SUPPRIMER :
            return do_ASDCEVT_ABO_SUPPRIMER(vfxNum, asdcNum, statics, arg);

        case ASDC_ABO_LIBERER :
            return do_ASDC_ABO_LIBERER(vfxNum, asdcNum, statics, arg);

        case ASDCEVT_CC_AJOUTER :
            return do_ASDCEVT_CC_AJOUTER(vfxNum, asdcNum, statics, arg);

        case ASDCEVT_CC_SUPPRIMER :
            return do_ASDCEVT_CC_SUPPRIMER(vfxNum, asdcNum, statics, arg);

        case ASDC_ABO_MF :
            return do_ASDC_ABO_MF(vfxNum, asdcNum, statics, arg);

        case ASDC_ABO_MFX :
            return do_ASDC_ABO_MFX(vfxNum, asdcNum, statics, arg);

        case ASDC_ABO_GF :
            return do_ASDC_ABO_GF(vfxNum, asdcNum, statics, arg);

        case ASDC_ABO_IV :
            return do_ASDC_ABO_IV(vfxNum, asdcNum, statics, arg);

        case ASDC_ABO_VV :
            return do_ASDC_ABO_VV(vfxNum, asdcNum, statics, arg);

        case ASDC_ABO_GV :
            return do_ASDC_ABO_GV(vfxNum, asdcNum, statics, arg);

        case ASDC_RT_GETTREP :
            return do_ASDC_RT_GETTREP(vfxNum, asdcNum, statics, arg);

        case ASDC_RT_SETTREP :
            return do_ASDC_RT_SETTREP(vfxNum, asdcNum, statics, arg);

        case ASDC_RT_GETSTS :
            return do_ASDC_RT_GETSTS(vfxNum, asdcNum, statics, arg);

        case ASDC_RT_SETSTS :
            return do_ASDC_RT_SETSTS(vfxNum, asdcNum, statics, arg);

        case ASDC_CC_GETDATA :
            return do_ASDC_CC_GETDATA(vfxNum, asdcNum, statics, arg);

        case ASDC_CC_SETDATA :
            return do_ASDC_CC_SETDATA(vfxNum, asdcNum, statics, arg);

        case ASDC_SA_GETNBM :
            return do_ASDC_SA_GETNBM(vfxNum, asdcNum, statics, arg);

        case ASDC_SA_SETNBM :
            return do_ASDC_SA_SETNBM(vfxNum, asdcNum, statics, arg);

        case ASDC_SA_LIER :
            return do_ASDC_SA_LIER(vfxNum, asdcNum, statics, arg);

        case ASDC_SA_DELIER :
            return do_ASDC_SA_DELIER(vfxNum, asdcNum, statics, arg);

        case ASDCLVER :
            return do_ASDCLVER(vfxNum, asdcNum, statics, arg);

        default :

           // printk("ASDC ioctl(0x%X) ==> passage au suivant\n", cmd);
           // printk("ASDC cevt_ioctl=%p\n", cevt_ioctl);

           /* Appel de la fonction ioctl du CEVT si elle existe */
           if (cevt_ioctl) {
               return (*cevt_ioctl)(vfxNum, asdcNum, fp, cmd, arg);
           }

            printk("VFX70(%d) - ASDC(%d) - CEVT deconnecte : ioctl(%d) "
                   "non definie !\n",vfxNum, asdcNum, cmd);
            return -EINVAL;
    }

    /* Erreur qui ne peut pas arriver !!! */
    printk("asdc_ioctl : Ce message n'aurait jamais du etre ecrit !\n");
    return -ENOTTY;
}


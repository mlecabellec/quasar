

/*
{+D}
   SYSTEM:          Software for Pmcvfx

   MODULE NAME:     isr_pmcvfx.c - interrupt handler

   VERSION:         A

   CREATION DATE:   04/01/09

   CODED BY:        FM

   ABSTRACT:        This routine performs interrupt exception handling
                    for interrupts on the Pmcvfx I/O board.

   CALLING
	SEQUENCE:   This subroutine runs as a result of an interrupt occuring.

   MODULE TYPE:

   I/O RESOURCES:

   SYSTEM
	RESOURCES:

   MODULES
	CALLED:

   REVISIONS:

 DATE      BY       PURPOSE
---------- ---- ---------------------------------------------------------------
 5/04/2013  YG  Adaptation a un driver ASDC place au-dessus du driver VFX70
17/04/2013  YG  Correction de l'acces aux registres materiels
16/07/2014  YG   Suppression inclusion inutile de fichiers <asm/xxx.h>

{-D}
*/

/*
   MODULES FUNCTIONAL DETAILS:
*/

#ifndef BUILDING_FOR_KERNEL
#define BUILDING_FOR_KERNEL	/* controls conditional inclusion in file pmcvxmulticommon.h */
#endif


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/interrupt.h>


//#include "../h/pmcvxmulticommon.h"
#include "vfx_ctl.h"
#include "vfxvarg.h"

/* Renvoi 0 si une fonction de traitement a bien ete appelee */
/* Renvoi 1 si aucune fonction de traitement n'a ete trouvee */
int isr_pmcvfx(int unit)

{

/*
	Local data areas
*/

volatile word type, enable, status;
volatile int  j;						/* loop control */
volatile word b_mask;					/* bit interrupt bit mask */
volatile struct mapvfx *map_ptr;		/* pointer to board memory map */

    int output;    /* Compte rendu renvoye */

/*
	Entry point of routine:
*/

#define L2(a) readl((int *)(pvargs[unit]->base2 + (a)))
#define E2(a, v) writel((v), (int *)(pvargs[unit]->base2 + (a)))
    /* Instrumentation pour mesures perfos : descente d'une voie TOR */
    /* Mise a un de la sortie TOR SSYNC3                        */
    /* E2(0x9094, L2(0x9094) | 0x00000004);                     */


    output = 0;    /* Valeur de retour OK a priori */

	/*  Initialize pointers to h/W registers*/
        map_ptr = (volatile struct mapvfx *) pvargs[unit]->base2;

	status = ( readw( (word*)&map_ptr->sts_reg) & 0xFF ); /* interrupt status */

// printk("status = 0x%X\n", status);

	if( status )		/* bits 0 - 7 are digital non-zero if DIO interrupt pending */
	{
	 	type = readw( (word*)&map_ptr->type_reg); /* interrupt type */

	 	enable = readw( (word*)&map_ptr->en_reg); /* interrupt enable */

/// printk("\nDIO ISR HwdAdr=%lX t=%X e=%X s=%X\n",
///       (unsigned long)map_ptr, type, enable, status);

	    /* Check each bit of the status register for a interrupt pending */
	    for( j = 0; j < 8; j++ )		/* prioritization bit 0 to 7 */
  	    {   
	        b_mask = 1 << j;		/* form interrupting mask */
	        if( b_mask & status ) /* non-zero if interrupt pending */
	        {
		     if( b_mask & type )  /* non-zero if COS selected */
		     {  
                         printk("j=%d type=0x%02X : COS IT\n", j, type);
/*					
                   Change of State (COS) interrupts are used when the user wants interrupt
                   processing to be done each time the state of the interrupt input signal
                   changes from high to low or from low to high.
                   The user can place a service routine here. 
*/
		     }
		     else				     /* LEVEL selected */
		     {
/*                 Disable Interrupt (DBI) approach is used when the interrupting input stimulus
                   CANNOT be removed by the interrupt handlers. This is the case when the interrupt
                   handler cannot cause the interrupt input to return to its original state again
                   by communicating with the interrupting device. This may be because communication
                   with the interrupt device is not direct or because the interrupting device cannot
                   respond by clearing its interrupting signal quickly enough.
                   The user may change the polarity setting in the Interrupt Polarity register so
                   that the interrupt occurs ONLY when digital input signal goes to the polarity
                   level selected.  The interrupt input point is not going to cause interrupts
                   anymore until interrupts for that point are re-enabled.
                   The user can place a service routine here. 
*/
                   //      printk("VFX70_IT : j=%d type=0x%02X : Lvl IT\n", j, type);

                         /* Appel de l'ISR utilisateur si elle existe */
                         if (pvargs[unit]->up_isr) {
                             (*pvargs[unit]->up_isr)(pvargs[unit]->userNum);
                         } else {
                                /* Si l'ISR utilisateur n'existe pas, l'IT */
                                /* ne sera pas acquittee (c'est le role de */
                                /* cette ISR) et on renvoi 1 (dans output) */
                                /* pour demander au driver de maintenir    */
                                /* les interruptions inhibees pour ne pas  */
                                /* bloquer le systeme                      */
                                output = 1;  /* Acquittement impossible */
                         }

                         /* Si la source de l'interruption est bien      */
                         /* le coupleur ASDC, cette interruption devrait */
                         /* maintenant etre acquittee.                   */

			 enable &= (~b_mask); /* disable this bit */
		     }
		}
            }

            writew( enable, (word*)&map_ptr->en_reg);
            writew( status, (word*)&map_ptr->sts_reg);/* clear pending interrupts */

	}

    /* Si output = 0, on rearme l'interruptions ASDC */
    /*  TODO : Devrait sans doute etre deplace ailleurs...         */
    /*         Maintenu ici pour aller vite !!! (YG, le 18/4/2013) */
    writel(1, (int *)(pvargs[unit]->base2 + 0x8014));

    /* Instrumentation pour mesures perfos : descente d'une voie TOR */
    /* Mise a un de la sortie TOR SSYNC4                        */
    /* E2(0x9094, L2(0x9094) | 0x00000008);                     */

    return output;
}

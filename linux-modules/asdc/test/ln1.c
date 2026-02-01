/************************************************************************
 * File             : ln1.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * History
 *
 * Edition Date     Comment                                        By
 * ------- -------- ---------------------------------------------- ---
 *       1  2/04/01 created                                        yg
 *
 *       2 26/09/02 - Passage en unsigned retour LL_get_ram 
 *		    - Prise en compte instrumentation (champ
 *		      device) de ioctl(sbs_1553_read)              yg
 *
 *       3 15/02/08 - Mise en conformite gcc v4                    yg
 *
 * NAME:  ln1.c
 *
 * DESCRIPTION:
 *    Initialement : extrait de ll_lynx.c des seules fonctions
 *    simultanement fonctionnelles et utiles.
 *    Depuis : interface vers les appels ioctl les plus courants
 *
 * FUNCTIONS:
 *    LL_get_ram()
 *    LL_init_shared_memory()
 *    LL_put_ram()
 *    LL_lire_16()
 *    LL_ecrire_16()
 *
 *====================================================================*/


#include <stdio.h>
#include <stdlib.h>
//#include <smem.h>
//#include <file.h>
#include <fcntl.h>
#include <sys/types.h>


/* -- SBS Driver interface structure -- */
#include "asdcctl.h"



/*====================================================================*/


/*======================================================================
 * NAME:  LL_get_ram()
 *
 * DESCRIPTION:
 *   This function fetches a 16-bit value from the SBS device.
 *
 * ARGUMENTS:
 *   poignee : Identificateur obtenu par open()
 *   offset : base offset to retrieve
 *
 * RETURN VALUE:
 *   short : value found at base offset
 *
 *====================================================================*/

unsigned short LL_get_ram( int poignee, long offset)
{
   short buffer;

   struct sbs1553_ioctl ioctl_arguments;
   
   if ((offset < 0) || (offset > 0x40000))
     { fprintf(stderr, "LL_get_ram (poignee=%d) - Offset=0x%X : hors bornes\n",
                        poignee, offset);
       exit(-1);
     }

   ioctl_arguments.buffer = &buffer;
   ioctl_arguments.length = 1;
   ioctl_arguments.offset = offset;
   
   /* Instrumentation ioctl(sbs1553_read) pour detecter les extensions	*/
   /* de signe intempestives :						*/
   /*   le code "position=101" identifie l'utilisation de cette ioctl	*/
   /* dans les fonctions utilitaires de niveau 1.			*/
   ioctl_arguments.device = 101;

   /* printf("Avant l'appel de ioctl(%x [0x%0X])\n",      */
   /*        sbs1553_read, sbs1553_read);                 */
   
   /* Read short from memory */
   if ( ioctl( poignee, sbs1553_read, (char *)&ioctl_arguments ) == -1 )
   {
   	printf("ll_get_ram ioctl call failed...");
        perror("ioctl ");
   }
  
   /* printf("GET_RAM read 0x%x a l'adresse 0x%x\n", buffer, offset ); */

   return buffer;
}

/*====================================================================*/






/*======================================================================
 * NAME:  LL_init_shared_memory()
 *
 * DESCRIPTION:
 *   This function fetches a shared memory segment from the operating
 *   system and overlays it at the location in physical memory at which
 *   the board resides. This physical memory location is mapped to a
 *   virtual memory space recognized by the operating system.  All input
 *   and output to the SBS device is through this virtual memory space.
 *
 * ARGUMENTS:
 *   poignee : Identificateur obtenu par open()
 *
 * RETURN VALUE:
 *   (short *) pointer to virtual memory space
 *
 *====================================================================*/

short *LL_init_shared_memory(int poignee)

{
   struct sbs1553_ioctl ioctl_arguments;
   char swp_array[10];
   char *swp_ptr;
   unsigned long base_address;

   ioctl_arguments.buffer = (short *)(&base_address);
   ioctl_arguments.length = 4;
   ioctl_arguments.offset = 0;

   printf("\nLL_init_shared_memory... getting base address!");

   if ( ioctl(poignee, sbs1553_get_baseaddr, &ioctl_arguments) == -1 )
     {
   	printf("Get Base address ioctl call failed...");
     }

  /* printf("\nAddress returned %X > ", base_address ); */

   /* swap the address to fixup */
   swp_ptr = (char *) &base_address;
   swp_array[0] = *swp_ptr++;
   swp_array[1] = *swp_ptr++;
   swp_array[2] = *swp_ptr++;
   swp_array[3] = *swp_ptr;   

   swp_ptr = (char *) &base_address;
   *swp_ptr++ = swp_array[1];
   *swp_ptr++ = swp_array[3];
   *swp_ptr++ = swp_array[2];
   *swp_ptr = swp_array[0];

   /* printf(" ADDRESS returned %X > ", base_address ); */
 
   return (short *) base_address;

}

/*====================================================================*/




/*======================================================================
 * NAME:  LL_put_ram()
 *
 * DESCRIPTION:
 *   This function writes a 16-bit value from the SBS device.
 *
 * ARGUMENTS:
 *   poignee : Identificateur obtenu par open()
 *   offset : base offset to retrieve
 *   value : 16-bit value to write to base offset
 *
 * RETURN VALUE:  (none)
 *
 *====================================================================*/

void LL_put_ram(int poignee, long offset, short value )

{
   short  buffer;
   struct sbs1553_ioctl ioctl_arguments;

   buffer = value;

   ioctl_arguments.buffer = &buffer;
   ioctl_arguments.length = 1;
   ioctl_arguments.offset = offset;

   // printf("PUT_RAM writing 0x%x a l'adresse 0x%x\n", value, offset ); 

   /* Write USHORT from memory */

   if (ioctl(poignee, sbs1553_write, &ioctl_arguments) == -1)
   {
   	printf("ll_put_ram ioctl call failed...");
   }

}

/*====================================================================*/


/*======================================================================
 * NAME:  LL_lire_16()
 *
 * DESCRIPTION:
 *   Lecture DIRECTE d'un mot (16 bits) sur le bus PCI
 *
 * ARGUMENTS:
 *   poignee : Identificateur obtenu par open()
 *   offset : base offset to retrieve
 *
 * RETURN VALUE:
 *   short : value found at base offset
 *
 *====================================================================*/

unsigned short LL_lire_16( int poignee, long offset )

{
   short buffer;

   struct sbs1553_ioctl ioctl_arguments;

   ioctl_arguments.buffer = &buffer;
   ioctl_arguments.length = 1;
   ioctl_arguments.offset = offset;

   /* Read short from memory */

   if ( ioctl( poignee, ABI_LIRE_16, (char *)&ioctl_arguments ) == -1 )
   {
   	printf("ll_lire_16 ioctl call failed...");
   }

   return buffer;
}

/*====================================================================*/


/*======================================================================
 * NAME:  LL_ecrire_16()
 *
 * DESCRIPTION:
 *   Ecriture DIRECTE d'un mot (16 bits) sur le bus PCI
 *
 * ARGUMENTS:
 *   poignee : Identificateur obtenu par open()
 *   offset : base offset to retrieve
 *   value : 16-bit value to write to base offset
 *
 * RETURN VALUE:  (none)
 *
 *====================================================================*/

void LL_ecrire_16(int poignee, long offset, short value )

{
   short  buffer;
   struct sbs1553_ioctl ioctl_arguments;

   buffer = value;

   ioctl_arguments.buffer = &buffer;
   ioctl_arguments.length = 1;
   ioctl_arguments.offset = offset;

   if (ioctl(poignee, ABI_ECRIRE_16, &ioctl_arguments) == -1)
   {
   	printf("ll_ecrire_16 ioctl call failed...");
   }

}

/*====================================================================*/


/*======================================================================
 * NAME:  LL_lire_32()
 *
 * DESCRIPTION:
 *   Lecture DIRECTE d'un mot (32 bits) sur le bus PCI
 *
 * ARGUMENTS:
 *   poignee : Identificateur obtenu par open()
 *   offset : base offset to retrieve
 *
 * RETURN VALUE:
 *   int : value found at base offset
 *
 *====================================================================*/

int LL_lire_32( int poignee, long offset )

{
   int buffer;

   struct sbs1553_ioctl ioctl_arguments;

   ioctl_arguments.buffer = (short *) &buffer;
   ioctl_arguments.length = 1;
   ioctl_arguments.offset = offset;

   /* Read short from memory */

   if ( ioctl( poignee, ABI_LIRE_32, (char *)&ioctl_arguments ) == -1 )
   {
   	printf("ll_lire_32 ioctl call failed...");
   }

   return buffer;
}

/*====================================================================*/


/*======================================================================
 * NAME:  LL_ecrire_32()
 *
 * DESCRIPTION:
 *   Ecriture DIRECTE d'un mot (32 bits) sur le bus PCI
 *
 * ARGUMENTS:
 *   poignee : Identificateur obtenu par open()
 *   offset : base offset to retrieve
 *   value : 32-bit value to write to base offset
 *
 * RETURN VALUE:  (none)
 *
 *====================================================================*/

void LL_ecrire_32(int poignee, long offset, int value )

{
   int  buffer;
   struct sbs1553_ioctl ioctl_arguments;

   buffer = value;

   ioctl_arguments.buffer = (short *) &buffer;
   ioctl_arguments.length = 1;
   ioctl_arguments.offset = offset;

   if (ioctl(poignee, ABI_ECRIRE_32, &ioctl_arguments) == -1)
   {
   	printf("ll_ecrire_32 ioctl call failed...");
   }

}


/*=========================================================*/




/*======================================================================
 * NAME:  LL_get_image()
 *
 * DESCRIPTION:
 *   This function fetches a 32-bit value from the "image static memory"
 *
 * ARGUMENTS:
 *   poignee : Identificateur obtenu par open()
 *   offset : base offset to retrieve
 *
 * RETURN VALUE:
 *   short : value found at base offset
 *
 *====================================================================*/

long LL_get_image( int poignee, long offset )

{
   long buffer;

   struct sbs1553_ioctl ioctl_arguments;

   ioctl_arguments.buffer = (short *) &buffer;
   ioctl_arguments.length = 1;
   ioctl_arguments.offset = offset;

   /* Read long from image */

   if ( ioctl( poignee, ASDC_IMAGE_LIRE, (char *)&ioctl_arguments ) == -1 )
   {
   	perror("ll_get_image ioctl call failed ");
   }
  
   /* printf("GET_IMAGE read %x", buffer ); */

   return buffer;
}

/*====================================================================*/




/*======================================================================
 * NAME:  LL_put_image()
 *
 * DESCRIPTION:
 *   This function writes a 32-bit value to the "static image memory".
 *
 * ARGUMENTS:
 *   poignee : Identificateur obtenu par open()
 *   offset : base offset to retrieve
 *   value : 32-bit value to write to base offset
 *
 * RETURN VALUE:  (none)
 *
 *====================================================================*/

void LL_put_image(int poignee, long offset, long value )

{
   long  buffer;
   struct sbs1553_ioctl ioctl_arguments;

   buffer = value;

   ioctl_arguments.buffer = (short *) &buffer;
   ioctl_arguments.length = 1;
   ioctl_arguments.offset = offset;

/* printf("PUT_RAM writing %x ", value ); */

   /* Write USHORT from memory */

   if (ioctl(poignee, ASDC_IMAGE_ECRIRE, &ioctl_arguments) == -1)
   {
   	perror("LL_put_image ioctl call failed ");
   	exit(-1);
   }

}

/*====================================================================*/



/*======================================================================
 * NAME:  LL_get_image_l()
 *
 * DESCRIPTION:
 *   
 *
 * ARGUMENTS:
 *   poignee : Identificateur obtenu par open()
 *   offset : base offset to retrieve
 *
 * RETURN VALUE:
 *   short : value found at base offset
 *
 *====================================================================*/

unsigned short LL_get_image_l( int poignee, long offset)
{
   short buffer;

   struct sbs1553_ioctl ioctl_arguments;
   
   if ((offset < 0) || (offset > 0x65535))
     { fprintf(stderr, "LL_get_ram (poignee=%d) - Offset=0x%X : hors bornes\n",
                        poignee, offset);
       exit(-1);
     }

   ioctl_arguments.buffer = &buffer;
   ioctl_arguments.length = 1;
   ioctl_arguments.offset = offset;
   
   /* Instrumentation ioctl(ASDC_IMAGE_LIRE_L) pour detecter les extensions	*/
   /* de signe intempestives :						*/
   /*   le code "position=101" identifie l'utilisation de cette ioctl	*/
   /* dans les fonctions utilitaires de niveau 1.			*/
   ioctl_arguments.device = 101;

   /* Read short from memory */

   if ( ioctl( poignee, ASDC_IMAGE_LIRE_L, (char *)&ioctl_arguments ) == -1 )
   {
   	printf("ll_get_ram ioctl call failed...");
   }
  
   /* printf("GET_RAM read 0x%x a l'adresse 0x%x\n", buffer, offset ); */

   return buffer;
}

/*====================================================================*/


/*======================================================================
 * NAME:  LL_get_image_h()
 *
 * DESCRIPTION:
 *   
 *
 * ARGUMENTS:
 *   poignee : Identificateur obtenu par open()
 *   offset : base offset to retrieve
 *
 * RETURN VALUE:
 *   short : value found at base offset
 *
 *====================================================================*/

unsigned short LL_get_image_h( int poignee, long offset)
{
   short buffer;

   struct sbs1553_ioctl ioctl_arguments;
   
   if ((offset < 0) || (offset > 0x65535))
     { fprintf(stderr, "LL_get_ram (poignee=%d) - Offset=0x%X : hors bornes\n",
                        poignee, offset);
       exit(-1);
     }

   ioctl_arguments.buffer = &buffer;
   ioctl_arguments.length = 1;
   ioctl_arguments.offset = offset;
   
   /* Instrumentation ioctl(ASDC_IMAGE_LIRE_H) pour detecter les extensions	*/
   /* de signe intempestives :						*/
   /*   le code "position=101" identifie l'utilisation de cette ioctl	*/
   /* dans les fonctions utilitaires de niveau 1.			*/
   ioctl_arguments.device = 101;

   /* Read short from memory */

   if ( ioctl( poignee, ASDC_IMAGE_LIRE_H, (char *)&ioctl_arguments ) == -1 )
   {
   	printf("ll_get_ram ioctl call failed...");
   }
  
   /* printf("GET_RAM read 0x%x a l'adresse 0x%x\n", buffer, offset ); */

   return buffer;
}

/*====================================================================*/



/*======================================================================
 * NAME:  LL_put_image_l()
 *
 * DESCRIPTION:
 *   This function writes a 16-bit value from the SBS device.
 *
 * ARGUMENTS:
 *   poignee : Identificateur obtenu par open()
 *   offset : base offset to retrieve
 *   value : 16-bit value to write to base offset
 *
 * RETURN VALUE:  (none)
 *
 *====================================================================*/

void LL_put_image_l(int poignee, long offset, short value )

{
   short  buffer;
   struct sbs1553_ioctl ioctl_arguments;

   buffer = value;

   ioctl_arguments.buffer = &buffer;
   ioctl_arguments.length = 1;
   ioctl_arguments.offset = offset;

   // printf("PUT_RAM writing 0x%x a l'adresse 0x%x\n", value, offset ); 

   /* Write USHORT from memory */

   if (ioctl(poignee, ASDC_IMAGE_ECRIRE_L, &ioctl_arguments) == -1)
   {
   	printf("ll_put_ram ioctl call failed...");
   }

}

/*====================================================================*/



/*======================================================================
 * NAME:  LL_put_image_h()
 *
 * DESCRIPTION:
 *   
 *
 * ARGUMENTS:
 *   poignee : Identificateur obtenu par open()
 *   offset : base offset to retrieve
 *   value : 16-bit value to write to base offset
 *
 * RETURN VALUE:  (none)
 *
 *====================================================================*/

void LL_put_image_h(int poignee, long offset, short value )

{
   short  buffer;
   struct sbs1553_ioctl ioctl_arguments;

   buffer = value;

   ioctl_arguments.buffer = &buffer;
   ioctl_arguments.length = 1;
   ioctl_arguments.offset = offset;

   // printf("PUT_RAM writing 0x%x a l'adresse 0x%x\n", value, offset ); 

   /* Write USHORT from memory */

   if (ioctl(poignee, ASDC_IMAGE_ECRIRE_H, &ioctl_arguments) == -1)
   {
   	printf("ll_put_ram ioctl call failed...");
   }

}

/*====================================================================*/


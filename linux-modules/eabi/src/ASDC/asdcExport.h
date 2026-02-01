/************************************************************************
 *                                                                      *
 *      Driver pour coupleur 1553 EMUABI                                *
 *     ----------------------------------                               *
 *                                                                      *
 *     Prototypes des fonctions exportees par le driver ASDC            *
 *      au profit du driver CEVT.                                       *
 *                                                                      *
 *                                                                      *
 *  Version initiale :                     Y.Guillemot, le 11/04/2013   *
 *  Modification types pour compatibilite 32b/64b       le 18/06/2014   *
 ************************************************************************/


#ifndef ASDC_EXPORT_H_
#define ASDC_EXPORT_H_

#include <linux/types.h>


/* Fonctions d'enregistrement/desenregistrement du driver CEVT */
/* aupres du driver ASDC                                       */

extern int
ASDC_registerCEVTDriver(long (* ioctl)(int vfx, int asdc, struct file *fp,
                                       unsigned int cmd, unsigned long arg),
                        int (* existence)(int),
                        int (* signaler)(int, int, int, int, int32_t, int32_t),
                        int (* signaler_date)(int, int, int, int, int32_t,
                                              int32_t, unsigned long long));


extern int
ASDC_unregisterCEVTDriver(void);


#endif  /* ASDC_EXPORT_H_ */

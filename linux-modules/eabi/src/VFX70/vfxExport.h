/************************************************************************
 *                                                                      *
 *      Driver pour carte Acromag VFX70                                 *
 *     ---------------------------------                                *
 *                                                                      *
 *     Prototypes des fonctions exportees (appelees par le driver       *
 *     utilisateur.                                                     *
 *                                                                      *
 *                                                                      *
 *  Version initiale :                     Y.Guillemot, le  5/04/2013   *
 *  Ajout lectureRamPCIe et ecritureRamPCIe             le 11/02/2015
 ************************************************************************/


#ifndef VFXEXPORT_H_
#define VFXEXPORT_H_


int VFX70_getNumberOfCards(void);

int32_t VFX70_getApplicationDescriptor(unsigned int vfxUnit);

int VFX70_registerUserDriver(unsigned int vfxNum, unsigned int userNum,
                             long (* up_ioctl)(int vfx, int asdc,
                                               struct file *fp,
                                               unsigned int cmd,
                                               unsigned long arg),
                             void (* up_isr)(int unit));

void VFX70_getBaseAddresses(unsigned int vfxNum, void ** pcibar0,
                            void ** pcibar1, void ** pcibar2);

int VFX70_unregisterUserDriver(unsigned int vfxNum);



unsigned int lecturePCIe(int vfxNum, int it, void * adresse);
void ecriturePCIe(int vfxNum, int it, unsigned int valeur, void * adresse);

unsigned int lectureRamPCIe(int vfxNum, int it, unsigned int adresse);
void ecritureRamPCIe(int vfxNum, int it, unsigned int valeur, unsigned int adresse);

#endif  /* VFXEXPORT_H_ */

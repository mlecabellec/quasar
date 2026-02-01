/************************************************************************
 *                                                                      *
 *      Driver pour carte Acromag VFX70                                 *
 *     ---------------------------------                                *
 *                                                                      *
 *                     VARIABLES GLOBALES DU PILOTE                     *
 *                                                                      *
 *                                                                      *
 *  Version initiale :                     Y.Anonymized, le  4/04/2013   *
 ************************************************************************/


#ifndef VFXVARG_H_
#define VFXVARG_H_


typedef struct vfxvarg_s {

    /* Le numero du device VFX70 */
    int unit;

    /* Le numero de device du driver utilisateur */
    int userNum;

    /* Les pointeurs vers la partie deportee des fonctions */
    void (* up_isr)(int unit);
    long (* up_ioctl)(int vfxNum, int userNum,
                      struct file *fp, unsigned int cmd, unsigned long arg);

    unsigned long base0;     /* Adresse de base du PCI_BAR0   */
    unsigned long base1;     /* Adresse de base du PCI_BAR1   */
    unsigned long base2;     /* Adresse de base du PCI_BAR2   */
    
} vfxvarg_t;


extern int open_dev[MAX_PMCS];
extern unsigned int board_irq[MAX_PMCS];
extern unsigned long pmc_address[MAX_PMCS];
extern unsigned long pmc_address1[MAX_PMCS];
extern unsigned long pmc_address2[MAX_PMCS];
extern struct pci_dev *pvfxBoard[MAX_PMCS];
extern vfxvarg_t *pvargs[MAX_PMCS];      /* Table des structures statiques */


#endif  /* VFXVARG_H_ */

/**************************************************************************
 *                                                                        *
 *      Driver pour carte NI PCIe-6509 (96 E/S TOR)                       *
 *     ---------------------------------------------                      *
 *                                                                        *
 * pcie6509ioctl.c : Fonction ioctl                                       *
 *                   (C'est ici que le "vrai" travail est fait)           *
 *                                                                        *
 *                                 ************ - ************ - ************
 *Anonymized
 **
 **************************************************************************/

/*
   QUAND    QUI   QUOI
---------- ----- ---------------------------------------------------------------
 3/12/2013   YG  Version initiale

*/

#include "DriverIncludes.h"

/* ----------------------------------------------------
   --                                                --
   -- Fonction ioctl                                 --
   --                                                --
   ---------------------------------------------------- */
long pcie6509UnlockedIoctl(struct file *fp, unsigned int cmd,
                           unsigned long arg) {
  int mineur;
  pcie6509Statics_t *pstat;
  int port;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
  mineur = MINOR(fp->f_dentry->d_inode->i_rdev);
#else
  mineur = MINOR(fp->f_path.dentry->d_inode->i_rdev);
#endif
  pstat = pAllStat[mineur];

  switch (cmd) {

  case PCIE6509_RAZ: {
    /* Remise du coupleur dans son etat initial : Toutes les voies */
    /* en position "entree".                                       */

    int i;

    spin_lock(&pstat->lock);
    for (i = 0; i < 12; i++) {
      /* Initialisation memoires */
      pstat->dir[i] = 0; /* 0 ==> Entree (1 ==> Sortie) */
      pstat->bit[i] = 0; /* 0 en sortie */

      /* Initialisation du materiel associe */
      writel(0, pstat->bvba + DIO_DIRECTION_P0_P3);
      writel(0, pstat->bvba + DIO_DIRECTION_P4_P5);
      writel(0, pstat->bvba + DIO_DIRECTION_P6_P7);
      writel(0, pstat->bvba + DIO_DIRECTION_P8_P11);
      writel(0, pstat->bvba + DIO_OUTPUT_P0_P3);
      writel(0, pstat->bvba + DIO_OUTPUT_P4_P5);
      writel(0, pstat->bvba + DIO_OUTPUT_P6_P7);
      writel(0, pstat->bvba + DIO_OUTPUT_P8_P11);
    }
    spin_unlock(&pstat->lock);

    return -OK;
  }

  case PCIE6509_GET_VDIR: {
    /* Lecture de la direction d'une voie */

    bit_t data;

    /* Copie des donnees depuis l'espace utilisateur */
    if (copy_from_user(&data, (bit_t *)arg, sizeof(data)))
      return -EFAULT;

    port = data.port;
    data.sens = (pstat->dir[port] >> data.bit) & 1;

    /* Copie des donnees vers l'espace utilisateur */
    if (copy_to_user((bit_t *)arg, &data, sizeof(data)))
      return -EFAULT;

    return -OK;
  }

  case PCIE6509_GET_PDIR: {
    /* Lecture de la direction des 8 voies d'un port */

    port_t data;

    /* Copie des donnees depuis l'espace utilisateur */
    if (copy_from_user(&data, (port_t *)arg, sizeof(data)))
      return -EFAULT;

    printLog(DRV_DEBUG, "ioctl(PCIE6509_GET_PDIR(%d))\n", data.port);

    port = data.port;
    data.data = pstat->dir[port] & 0xFF;

    /* Copie des donnees vers l'espace utilisateur */
    if (copy_to_user((bit_t *)arg, &data, sizeof(data)))
      return -EFAULT;

    return -OK;
  }

  case PCIE6509_SET_VDIR: {
    /* Modification de la direction d'une voie */

    bit_t data;
    uint8_t s;

    /* Copie des donnees depuis l'espace utilisateur */
    if (copy_from_user(&data, (bit_t *)arg, sizeof(data)))
      return -EFAULT;

    port = data.port;
    s = 1 << data.bit;

    spin_lock(&pstat->lock);
    if (data.sens & 1) {
      pstat->dir[port] |= s;
    } else {
      pstat->dir[port] &= ~s;
    }
    ecrirePortDirection(pstat, port, pstat->dir[port]);
    spin_unlock(&pstat->lock);

    return -OK;
  }

  case PCIE6509_SET_PDIR: {
    /* Modification de la direction des voies d'un port */

    port_t data;

    /* Copie des donnees depuis l'espace utilisateur */
    if (copy_from_user(&data, (port_t *)arg, sizeof(data)))
      return -EFAULT;

    port = data.port;

    spin_lock(&pstat->lock);
    pstat->dir[port] = data.data & 0xFF;
    ecrirePortDirection(pstat, port, pstat->dir[port]);
    spin_unlock(&pstat->lock);

    return -OK;
  }

  case PCIE6509_GET_BIT: {
    /* Lecture du niveau logique d'une voie */

    bit_t data;

    /* Copie des donnees depuis l'espace utilisateur */
    if (copy_from_user(&data, (bit_t *)arg, sizeof(data)))
      return -EFAULT;

    port = data.port;
    data.val = (lirePortEntree(pstat, port) >> data.bit) & 1;

    printLog(DRV_DEBUG, "PCIE6509_GET_BIT [%d,%d] : %d\n", port, data.bit,
             data.val);

    /* Copie des donnees vers l'espace utilisateur */
    if (copy_to_user((bit_t *)arg, &data, sizeof(data)))
      return -EFAULT;

    return -OK;
  }

  case PCIE6509_GET_BITS: {
    /* Lecture des niveaux logiques d'un port */

    port_t data;

    /* Copie des donnees depuis l'espace utilisateur */
    if (copy_from_user(&data, (port_t *)arg, sizeof(data)))
      return -EFAULT;

    printLog(DRV_DEBUG, "ioctl(PCIE6509_GET_BITS(%d))\n", data.port);

    port = data.port;
    data.data = lirePortEntree(pstat, port);

    {
      int a, b, c, d;
      a = readl(pstat->bvba + DIO_INPUT_P0_P3);
      b = readl(pstat->bvba + DIO_INPUT_P4_P5);
      c = readl(pstat->bvba + DIO_INPUT_P6_P7);
      d = readl(pstat->bvba + DIO_INPUT_P8_P11);
      printLog(DRV_DEBUG, "%08X %08X %08X %08X\n", a, b, c, d);
    }

    /* Copie des donnees vers l'espace utilisateur */
    if (copy_to_user((bit_t *)arg, &data, sizeof(data)))
      return -EFAULT;

    return -OK;
  }

  case PCIE6509_SET_BIT: {
    /* Modification du niveau logique d'une voie */

    bit_t data;
    uint8_t v;

    /* Copie des donnees depuis l'espace utilisateur */
    if (copy_from_user(&data, (bit_t *)arg, sizeof(data)))
      return -EFAULT;

    port = data.port;
    v = 1 << data.bit;

    printLog(DRV_DEBUG, "PCIE6509_SET_BIT [%d,%d] v=0x%X  0x%02X --> ", port,
             data.bit, v, pstat->bit[port]);

    spin_lock(&pstat->lock);
    if (data.val & 1) {
      pstat->bit[port] |= v;
    } else {
      pstat->bit[port] &= ~v;
    }

    printLog(DRV_DEBUG, "0x%02X\n", pstat->bit[port]);

    ecrirePortSortie(pstat, port, pstat->bit[port]);
    spin_unlock(&pstat->lock);

    return -OK;
  }

  case PCIE6509_SET_BITS: {
    /* Modification des niveaux logiques d'un port */

    port_t data;
    uint8_t v;

    /* Copie des donnees depuis l'espace utilisateur */
    if (copy_from_user(&data, (port_t *)arg, sizeof(data)))
      return -EFAULT;

    port = data.port;
    v = data.data & 0xFF;

    spin_lock(&pstat->lock);
    pstat->bit[port] = data.data & 0xFF;
    ecrirePortSortie(pstat, port, pstat->bit[port]);
    spin_unlock(&pstat->lock);

    return -OK;
  }

  case PCIE6509_GET_VER: {
    /* Lecture de la version du driver */

    version_t data;
    char *p;

    printLog(DRV_DEBUG, "ioctl(PCIE6509_GETVER)\n");

    /* Mise en forme et copie de la version du pilote */
    p = __stringify(PCIE6509_VERSION) "." __stringify(PCIE6509_REVISION);
    strcpy(data.version, p);

    /* Copie du nom du pilote */
    p = PCIE6509_NOM;
    strcpy(data.nom, p);

    /* Copie de la date du pilote */
    p = PCIE6509_DATE;
    strcpy(data.date, p);

    /* Copie des version et revision sous forme numerique */
    data.ver = PCIE6509_VERSION;
    data.rev = PCIE6509_REVISION;

    /* Copie des donnees vers l'espace utilisateur */
    if (copy_to_user((version_t *)arg, &data, sizeof(data)))
      return -EFAULT;

    return -OK;
  }

  default:
    /* Commande ioctl inconnue */
    return -ENOSYS;
  }

  printLog(DRV_ERROR, "pcie6509UnlockedIoctl: "
                      "Ce fragment de code n'aurait JAMAIS du etre execute\n");
  return -ENOTTY;
}

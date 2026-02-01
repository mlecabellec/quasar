/************************************************************************
 *                                                                      *
 *      Driver pour carte NI PCIe-6509 (96 E/S TOR)                     *
 *     ---------------------------------------------                    *
 *                                                                      *
 * pcie6509.h : Declaration de l'interface du driver.                   *
 *                                                                      *
 *                                 ************ - ************ - ************
 *Anonymized
 **
 ************************************************************************/

/*
   QUAND    QUI   QUOI
---------- ----- ---------------------------------------------------------------
 3/12/2013   YG  Version initiale
17/01/2014   YG  Ajout des lignes associes a la lecture de la version

*/

#ifndef PCIE6509_H_
#define PCIE6509_H_

/* Taille des chaines de caracteres utilisees pour copier les */
/* nom, version et revision du pilote.                        */
#define TMAXSTRVER 100

typedef struct bit_s {
  unsigned char port; /* Numero du port : 0 a 11 */
  unsigned char bit;  /* Numero du bit : 0 a 7 */
  unsigned char val;  /* Valeur associee au bit : 0 ou 1 */
  unsigned char sens; /* Sens (direction) de la voie : 0 ou 1 */
} bit_t;

typedef struct port_s {
  unsigned char port; /* Numero du port : 0 a 11 */
  unsigned char data; /* Valeur associee aux 8 bits du port */
} port_t;

typedef struct version_s {
  char nom[TMAXSTRVER + 1];     /* Nom du pilote */
  char version[TMAXSTRVER + 1]; /* Version.revision du pilote */
  char date[TMAXSTRVER + 1];    /* Date de revision du pilote */
  int ver;                      /* Version du pilote (forme numerique) */
  int rev;                      /* Revision du pilote (forme numerique) */
} version_t;

/*************************************/
/* Codes ioctl traites par le driver */
/*************************************/

#define MY_CMD_CODE 'G'
#define START 100

#define PCIE6509_RAZ _IO(MY_CMD_CODE, START + 1)

#define PCIE6509_GET_VDIR _IOWR(MY_CMD_CODE, START + 2, bit_t)
#define PCIE6509_SET_VDIR _IOW(MY_CMD_CODE, START + 3, bit_t)
#define PCIE6509_GET_BIT _IOWR(MY_CMD_CODE, START + 4, bit_t)
#define PCIE6509_SET_BIT _IOW(MY_CMD_CODE, START + 5, bit_t)

#define PCIE6509_GET_PDIR _IOWR(MY_CMD_CODE, START + 6, port_t)
#define PCIE6509_SET_PDIR _IOW(MY_CMD_CODE, START + 7, port_t)
#define PCIE6509_GET_BITS _IOWR(MY_CMD_CODE, START + 8, port_t)
#define PCIE6509_SET_BITS _IOW(MY_CMD_CODE, START + 9, port_t)

#define PCIE6509_GET_VER _IOR(MY_CMD_CODE, START + 10, port_t)

#define OK 0

#endif /* PCIE6509_H_ */

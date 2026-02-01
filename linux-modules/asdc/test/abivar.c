/*******************************************************************/
/*                                                                 */
/*   AFFICHAGE D'UNE PARTIE DE LA MEMOIRE ECHANGE DE L'AMI/ABI     */
/*           CAS D'UN ABI-V3 AVEC MICROCODE SBS modifie (V4.1)     */
/*                                                                 */
/*     Suppose que la fonction mmap() soit disponible dans le      */
/*     driver du peripherique concerne ...                         */
/*                                                                 */
/*              Anonymized,                                      */
/*                  Microcode SBS standard : le    18 avril 1994   */
/*                  Microcode SDC V4.1 :     le 8 septembre 1994   */
/*             Modif. ABIPMC2 et LynxOS :    le  31 juillet 2001   */
/*                           derniere modif. le      6 aout 2001   */
/*                Ajout RTSA_CUR et RTSA_BUF le     29 juin 2005   */
/*                 Mise en conformite gcc v4 le  15 fevrier 2008   */
/*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/file.h>

#include "asdcctl.h"
#include "ln1.h"



#define C	3	/* Nombre de colonnes pour affichage */

struct var { char *nom;
	     int adr;
	   };

/* Constitution de la table des variables et de leurs adresses */
struct var tbl[] =
      { { "MAP",	MAP         },	/* Sequencer MAP Register   */
	{ "RAM_ERR", 	RAM_ERR     },	/*         */
	{ "EncodErr",	ENCODER_ERR },	/*         */
	{ "Tst_Sel",	TEST_SELECT_REG },	/*         */
	{ "FWPROC",     FWPROC      },	/* Firmware product code    */
	{ "FWVERC",     FWVERC      },	/* Firmware version code    */
	{ "PCODE",      PCODE       },	/* Product code for DSP     */
	{ "VCODE",      VCODE       },	/* Version code for DSP     */
	{ "CNFGREG",    CNFGREG     },	/* Configuration register   */
	{ "CMD", 	CMD         },	/* Command Word             */
	{ "RESP", 	RESP        },	/* Response Word            */
	{ "IQRSP", 	IQRSP       },	/* It Queue Response Flag   */
	{ "IQPTR1", 	IQPTR1      },	/* It Queue Pointer 1       */
	{ "IQPTR2", 	IQPTR2      },	/* It Queue Pointer 2       */
	{ "IQCNT1", 	IQCNT       },	/* It Queue Count 1         */
	{ "IQCNT2",     IQCNTS      },	/* It Queue Count 2         */
	{ "IQNUM", 	IQNUM       },	/* It Queue Nbr of Entries  */
	{ "SWTPTR", 	SWTPTR      },	/* Status Word Table Ptr    */
	{ "ATPTR", 	ATPTR       },	/* Address Table Pointer    */
	{ "FTPTR", 	FTPTR       },	/* Filter Table Pointer     */
	{ "SFMS", 	SFMS        },	/*                          */
	{ "M1PTR", 	M1PTR       },	/* Monitor Buffer 1 Pointer */
	{ "M2PTR", 	M2PTR       },	/* Monitor Buffer 2 Pointer */
	{ "MBLEN ",	MBLEN       },	/* Monitor Buffer Length    */
	{ "BCIPTR", 	BCIPTR      },	/* BC Initial Pointer       */
	{ "BCCPTR", 	BCCPTR      },	/* BC Current Pointer       */
	{ "BCLPTR", 	BCLPTR      },	/* BC Last Block Pointer    */
	{ "BCSMSK", 	BCSMSK      },	/* BC Status Word Mask      */
	{ "SCHIGH", 	SCHIGH      },	/* System Clock Hight       */
	{ "SCLOW", 	SCLOW       },	/* System Clock Low         */
	{ "RTPPTR", 	RTPPTR      },	/* RT Phase Table Pointer   */
	{ "BITPTR", 	BITPTR      },	/* RT Bit Word Table Ptr    */
	{ "LCDPTR", 	LCDPTR      },	/* Last Command Table Ptr   */
	{ "TVWPTR", 	TVWPTR      },	/* Tr. Vector Word Tbl Ptr  */
	{ "LSWPTR", 	LSWPTR      },	/* Last Status Word Tbl Ptr */
	{ "LSYPTR", 	LSYPTR      },	/* Last Synch Word Tbl Ptr  */
	{ "PROPTR", 	PROPTR      },	/* Protocol Table Pointer   */
	{ "CCW", 	CCW         },	/* Clock Control Word       */
	{ "MIMPTR", 	MIMPTR      },	/* Mode It Mask Table Ptr   */
	{ "MBFLG", 	MBFLG       },	/* Seq. Mon. Control Word   */
	{ "SCHIGHI", 	SCHIGHI     },	/* IRIG clock hight         */
	{ "SCMIDI", 	SCMIDI      },	/* IRIG clock middle        */
	{ "SCLOWI", 	SCLOWI      },	/* IRIG clock low           */
	{ "BCERVL", 	BCERVL      },	/*   */
	{ "SMBCNT", 	SMBCNT      },	/*   */
	{ "RSPGPA", 	RSPGPA      },	/*   */
	{ "RSPGPS", 	RSPGPS      },	/*   */
	{ "BCIGP", 	BCIGP       },	/*   */
	{ "BRTCNT", 	BRTCNT      },	/*   */
	{ "BRTBUS", 	BRTBUS      },	/*   */
	{ "BRTCMD", 	BRTCMD      },	/*   */
	{ "BRTRTC", 	BRTRTC      },	/*   */
	{ "STUBSEL", 	STUBSEL     },	/*   */
	{ "MFPERFR",    MFPERFR     },	/*   */
	{ "MFTVALH",    MFTVALH     },	/*   */
	{ "MFTVALL",    MFTVALL     },	/*   */
	{ "MFTCNTH",    MFTCNTH     },	/*   */
	{ "MFTCNTL",    MFTCNTL     },	/*   */
	{ "MFTEXEH",    MFTEXEH     },	/*   */
	{ "MFTEXEL",    MFTEXEL     },	/*   */
	{ "ASYNCBC",    ASYNCBC     },	/*   */
	{ "ASYNCBCT",   ASYNCBCT    },	/*   */
	{ "BCPAM",      BCPAM       },	/*   */
	{ "VVCTL",      VVCTL       },	/*   */
	{ "VVALUE",     VVALUE      },	/*   */
	{ "GINT",      	GINT        },	/*   */
	{ "TRG3_CTL",   TRG3_CTL    },	/*   */
	{ "GLBLCTL",    GLBLCTL     },	/* Global control register  */
	{ "ERRTBL",     ERRTBL      },	/*                          */
	{ "INITOK", 	INITOK      },	/*                          */
	{ "RTSA_CUR",	RTSA_CUR    },  /* Transfert RTBC en cours  */
	{ "RTSA_BUF",	RTSA_BUF    },  /* Tampon en utilisation  */
      };

#define N	(sizeof(tbl) / sizeof(struct var))



main(argc,argv)
int argc;
char *argv[];
{
  int rslt;
  int fd;
  int decalage;   /* Position zone a afficher par rapport debut mem. periph. */
  union masdc *a;
  caddr_t uuu;

  int i, j, k, m, l, n, o;







  if (argc!=2) { printf("\nSyntaxe :   %s  periph. \n", argv[0]);
		 exit(-1);
	       }


  if ((fd=open(argv[1],O_RDWR)) < 0)
			 { perror("open");
			   printf("\n Impossible d'ouvrir '%s'\n",argv[1]);
			   exit(1);
			 }





  printf("ABI-PMC2 : \n");
  printf("\nVersion firmware (significatif seulement apres initialisation)\n");
  printf("Firmware : Version = %04X   Revision = %04X\n",
	  LRAM(fd, FWPROC) & 0xFFFF,
	  LRAM(fd, FWVERC) & 0xFFFF);
  printf("DSP code : Version = %04X   Revision = %04X\n",
	  LRAM(fd, PCODE) & 0xFFFF,
	  LRAM(fd, VCODE) & 0xFFFF);



  printf("\nContenu des \"registres de base\" de '%s' : \n", argv[1]);



  for (i=0; i<N/C+1; i++)
     { for (j=0; j<C; j++)
	  if ((k = i+j*(N/C+1)) < N)
	     { printf("%s[%04X] %-8s : 0x%04X",
	              j ? "   " : "",
		      tbl[k].adr & 0xFFFF,
		      tbl[k].nom,
		      LRAM(fd, tbl[k].adr) & 0xFFFF);
	     }
       printf("\n");
     }

  printf("\n");
  
  printf("CSR = 0x%04X\n", LRAM(fd, CSR) & 0xFFFF);


  printf("\nReglages courants :\n");
  printf("RSPGPA = %d us   RSPGPS = %d us    BCIGP = %d us\n",
	  LRAM(fd, RSPGPA)/4, LRAM(fd, RSPGPS)/4, LRAM(fd, BCIGP)/4);

  printf("MFTVAL = %u us   MFTCNT = %u   MFTEXE = %u\n",
	  ((LRAM(fd, MFTVALH) << 16) & 0xFFFF0000)
		      | (LRAM(fd, MFTVALL) & 0x0000FFFF),
	  ((LRAM(fd, MFTCNTH) << 16) & 0xFFFF0000)
		      | (LRAM(fd, MFTCNTL) & 0x0000FFFF),
	  ((LRAM(fd, MFTEXEH) << 16) & 0xFFFF0000)
		      | (LRAM(fd, MFTEXEL) & 0x0000FFFF));


  close(fd);
  exit(0);
}




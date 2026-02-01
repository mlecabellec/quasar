/*******************************************************************/
/*   MONABI : Logiciel d'espionnage 1553 utilisant une carte ABI   */
/*                                                                 */
/*      Affichage "en clair" du fichier brut fourni par MONABI     */
/*      ------------------------------------------------------     */
/*                                                                 */
/*             Lecture et decodage d'un bloc ESPION                */
/*                                                                 */
/*                                   Anonymized, le 16/12/1993   */
/*                                         modifie le  3/01/1994   */
/*                                         modifie le  6/01/1994   */
/*                                         modifie le  7/01/1994   */
/*                                         modifie le 13/01/1994   */
/*                                         modifie le 31/03/1994   */
/*                                                                 */
/*      Modifie pour bloc ESPION fourni par ABI-V3 le 17/06/1994   */
/*                                    Modifie V2.2 le  4/08/1994   */
/*                                                                 */
/*                     Modifie pour examen Pbs UES le 12/09/1994   */
/*                                                                 */
/*                   Modifie pour EXAM version 2.5 le 20/05/1997   */
/*                   Modifie pour EXAM version 2.6 le 21/05/1997   */
/*                      Modifie pour datation IRIG le  6/08/2001   */
/*******************************************************************/

#include <stdio.h>
#include "exam.h"

#define MASQUE_BUS2 1<<7	/* Bit du "type" a 1 si reception sur bus 2 */

examespionirig()
{  
   int nbmots, nbech;
   
   char temporaire[T_MAX_LIGNE];
   int lt;
   
   int type;
   int date1, date2, date3;
   int comm, comm2, d;
   char comm2_asc[10];
   int adr, sa, m, n, i, i0, j;
   int doinatt;		/* Indicateur donnees inattendues (spurious) */
   
   char *descr;
   int bus;
   int err;
   int nbr_mots;
   int msg_type;
   int nmb, neb;
   
   /* Pour permettre conversion date */
   char strdate[80];
   int us;
   
   
   
   
   
       nbmots = lec_mot(1);
       nbech = lec_mot(1);

       if (visutamp)
         { if (affderok) vidage_avant_dernier();

           printf("\nnbmots = %d\n", nbmots);     
           printf("nombre d'echanges = %d\n", nbech);
         }
       nbmots -= 2;
       
       if (nbech == 0xFFFF) 
         { if (affderok) vidage_avant_dernier();

           printf("*******************************\n");
           printf("*******   DEBORDEMENT   *******\n");
           printf("*******************************\n");
           printf("\n");
         }
       
       while(nbmots)
         { 
           if (nbmots < 0)
              { fprintf(stderr, "\nGros probleme !\n");
                fflush(stderr);
                abort();
              }
              
           type = lec_mot(1);
           nbmots--;

           msg_type = type & 0x7F;
           bus = type & MASQUE_BUS2 ? 2 : 1;
           err = (type >> 14) & 1;
           nbr_mots = (type >> 8) & 0x3F;
           doinatt = msg_type == 0x20;
           
           if (bus==1) nmb = ++nb_msg1; 
                  else nmb = ++nb_msg2;
           
           date1 = lec_mot(1);
           date2 = lec_mot(1);
           date3 = lec_mot(1);
           nbmots -= 3;
                            
                            
                            
           if (fok && !affderok && !err && !doinatt)
           {
           /*** Pas d'erreur et pas d'affichage des messages sans erreur ***/
           /***   ===> On parcours le tampon ABI sans mettre les donnees ***/
           /***        sous une forme humainement lisible.               ***/
           
           nb_msg++;
                  
           switch(msg_type)
              { case 1  : comm = lec_mot(1);
                          nbmots--;

                          sa = comm & 0x03E0;
                          m = comm & 0x1F;
                          switch(m)
                             { case 0 : ;
                               case 1 : ;
                               case 2 : ;
                               case 3 : ;
                               case 4 : ;
                               case 5 : ;
                               case 6 : ;
                               case 7 : ;
                               case 8 : i0 = 1; /* Nbre mots deja lus */
                                        n = 0;
                                        break;
                                        
                               case 0x10 : ;
                               case 0x12 : ;
                               case 0x13 : i0 = 1; /* Nbre mots deja lus */
                                           n = 1;
                                           break;
                             
                               case 0x11 : ;
                               case 0x14 : ;
                               case 0x15 : i0 = 1; /* Nbre mots deja lus */
                                           n = 1;
                                           break;
                                           
                               default : i0 = 1;  /* Nbre mots deja lus */
                                         n = 0; /* Et pourquoi pas ??? */
                             }
                          break;
                          
                case 2  : comm = lec_mot(1);
                          nbmots--;
                          
                          adr = (comm >> 11) & 0x1F;
                          sa = (comm >> 5) & 0x1F;
                          n = comm & 0x1F;
                          if (!n) n = 32;
                          i0 = 1;  /* Nbre mots deja lus */
                          break;
                          
                case 4  : comm = lec_mot(1);
                          nbmots--;
                          
                          adr = (comm >> 11) & 0x1F;
                          sa = (comm >> 5) & 0x1F;
                          n = comm & 0x1F;
                          if (!n) n = 32;
                          i0 = 1;  /* Nbre mots deja lus */
                          break;
                          
                case 8  : comm = lec_mot(1);
                          nbmots--;
                          
                          n = comm & 0x1F;
                          if (!n) n = 32;
                          comm2 = lec_mot(1);
                          nbmots--;
                          i0 = 2;  /* Nbre mots deja lus */
                          break;
                          
                case 0x10 : comm = lec_mot(1);
                          nbmots--;
 
                          n = comm & 0x1F;
                          if (!n) n = 32;
                          i0 = 1;  /* Nbre mots deja lus */
                          break;
                          
                case 0x18 : comm = lec_mot(1);
                          nbmots--;
 
                          n = comm & 0x1F;
                          if (!n) n = 32;
                          comm2 = lec_mot(1);
                          nbmots--;
                          i0 = 2;  /* Nbre mots deja lus */
                          break;
                          
                case 0x20 : n = nbr_mots - 4 - err;
                          i0 = 0;  /* Nbre mots deja lus */
                          nb_msg--;	/* On ne comptabilise pas le message */
                          spurious++;	/* On comptabilise le Pb */
                          
                          /* Idem pour la comptabilisation par bus */
                          if (bus==1) { nb_msg1--; 
                                        spurious1++;
                                      }
                                 else { nb_msg2--; 
                                        spurious2++; 
                                      }
                          
                          break;
                          
                default : fprintf(stderr,
                                  "type msg 0x%x inconnu (octet # 0x%lx) !\n",
                                  msg_type, octet_courant);
                          exit(-1);
              }
              
       
           for (i = j = 0; i < (nbr_mots - 4 - err - i0); i++, j++)
              { d = lec_mot(1);
                nbmots--;
              }

           }
           else
           {     
           /*** Erreur ou besoin possible d'affichage des messages sans ***/
           /*** erreur  ===> On parcours le tampon ABI en convertissant ***/
           /***            les donnees en un texte humainement lisible. ***/
           
           /* printf("date1,2,3 = 0x %04X %04X %04X\n",
                 date1 & 0xFFFF, date2 & 0xFFFF, date3 & 0xFFFF); */
                              
           us = ((date2 << 16) & 0xF0000) | (date3 & 0xFFFF);
           sprintf(strdate, "J%3d H%02d:%02d:%02d  %03d.%03d ms",
                   (date1 >> 5) & 0x1FF, date1 & 0x1F,
                   (date2 >> 10) & 0x3F, (date2 >> 4) & 0x3F,
                   us / 1000, us % 1000);
                   
           /* printf("us=%d    strdate=\"%s\"\n", us, strdate); */           
                   
           sprintf(temporaire, 
                   "\n%9d   B%d#%-9d   date = %s\n", 
                   ++nb_msg, bus, nmb, strdate);
           MAJTMP;
                      
           sprintf(temporaire, 
                   "type=0x%04X  msg_type=%02X Bus_id=%d Err=%d nbr_mots=%d\n",
                   type & 0xFFFF, msg_type, bus, err, nbr_mots);
           MAJTMP;
                  
           switch(msg_type)
              { case 1  : comm = lec_mot(1);
                          nbmots--;

                          sa = comm & 0x03E0;
                          if ((sa != 0) && (sa != 0x03E0)) descr = "???SA???";
                                                      else descr = "";
                          m = comm & 0x1F;
                          switch(m)
                             { case 0 : ;
                               case 1 : ;
                               case 2 : ;
                               case 3 : ;
                               case 4 : ;
                               case 5 : ;
                               case 6 : ;
                               case 7 : ;
                               case 8 : if (!(comm & 0x0400)) 
                                                  descr = "-BIZARRE-";
                                        i0 = 1; /* Nbre mots deja lus */
                                        n = 0;
                                        break;
                                        
                               case 0x10 : ;
                               case 0x12 : ;
                               case 0x13 : if (!(comm & 0x0400)) 
                                                  descr = "-BIZARRE-";
                                           i0 = 1; /* Nbre mots deja lus */
                                           n = 1;
                                           break;
                             
                               case 0x11 : ;
                               case 0x14 : ;
                               case 0x15 : if ((comm & 0x0400)) 
                                                  descr = "-BIZARRE-";
                                           i0 = 1; /* Nbre mots deja lus */
                                           n = 1;
                                           break;
                                           
                               default : descr = "-ANORMAL-";
                                         i0 = 1;  /* Nbre mots deja lus */
                                         n = 0; /* Et pourquoi pas ??? */
                             }
                          sprintf(temporaire, 
                                  "type = 0x%04x  -  comm = 0x%04x",
                                  type & 0xFFFF, comm & 0xFFFF);
                          MAJTMP;
                          sprintf(temporaire, 
                                  " -  n = %d     [CC]  %s Bus %d\n",
                                  n, descr, bus);                             
                          MAJTMP;
                          break;
                          
                case 2  : comm = lec_mot(1);
                          nbmots--;
                          
                          adr = (comm >> 11) & 0x1F;
                          sa = (comm >> 5) & 0x1F;
                          n = comm & 0x1F;
                          if (!n) n = 32;
                          sprintf(temporaire, 
                                  "type = 0x%04x  -  comm = 0x%04x",
                                  type & 0xFFFF, comm & 0xFFFF);
                          MAJTMP;
                          sprintf(temporaire, 
                                  " -  n = %d     [BC -> RT%d,%d]   Bus %d\n",
                                  n, adr, sa, bus);
                          MAJTMP;
                          i0 = 1;  /* Nbre mots deja lus */
                          break;
                          
                case 4  : comm = lec_mot(1);
                          nbmots--;
                          
                          adr = (comm >> 11) & 0x1F;
                          sa = (comm >> 5) & 0x1F;
                          n = comm & 0x1F;
                          if (!n) n = 32;
                          sprintf(temporaire, 
                                  "type = 0x%04x  -  comm = 0x%04x",
                                  type & 0xFFFF, comm & 0xFFFF);
                          MAJTMP;
                          sprintf(temporaire,
                                  " -  n = %d     [RT%d,%d -> BC]   Bus %d\n",
                                  n, adr, sa, bus);
                          MAJTMP;
                          i0 = 1;  /* Nbre mots deja lus */
                          break;
                          
                case 8  : comm = lec_mot(1);
                          nbmots--;
                          
                          n = comm & 0x1F;
                          if (!n) n = 32;
                          comm2 = lec_mot(1);
                          nbmots--;
                          sprintf(temporaire,
                                  "type = 0x%04x  -  comm = 0x%04x 0x%04x",
                                  type & 0xFFFF, comm & 0xFFFF, comm2 & 0xFFFF);
                          MAJTMP;
                          sprintf(temporaire, 
                                  " -  n = %d     [RT -> RT]   Bus %d\n",
                                  n, bus);
                          MAJTMP;
                          i0 = 2;  /* Nbre mots deja lus */
                          break;
                          
                case 0x10 : comm = lec_mot(1);
                          nbmots--;
 
                          n = comm & 0x1F;
                          if (!n) n = 32;
                          sprintf(temporaire,
                                  "type = 0x%04x  -  comm = 0x%04x -  n = ",
                                  type & 0xFFFF, comm & 0xFFFF);
                          MAJTMP;
                          sprintf(temporaire,
                                  "%d     [Diffusion BC -> RT]   Bus %d\n",
                                  n, bus);
                          MAJTMP;
                          i0 = 1;  /* Nbre mots deja lus */
                          break;
                          
                case 0x18 : comm = lec_mot(1);
                          nbmots--;
 
                          n = comm & 0x1F;
                          if (!n) n = 32;
                          comm2 = lec_mot(1);
                          nbmots--;
                          sprintf(temporaire,
                                  "type = 0x%04x  -  comm = 0x%04x 0x%04x -  n",
                                  type & 0xFFFF, comm & 0xFFFF, comm2 & 0xFFFF);
                          MAJTMP;
                          sprintf(temporaire,
                                  " = %d     [Diffusion RT -> RT]   Bus %d\n",
                                 n, bus);
                          MAJTMP;
                          i0 = 2;  /* Nbre mots deja lus */
                          break;
                          
                case 0x20 : n = nbr_mots - 4 - err;
                          sprintf(temporaire,
                                  "type = 0x%04x  -  n = ", type & 0xFFFF);
                          MAJTMP;
                          sprintf(temporaire,
                                  "%d     [Donnees inattendues]   Bus %d\n",
                                 n, bus);
                          MAJTMP;
                          i0 = 0;  /* Nbre mots deja lus */
                          nb_msg--;	/* On ne comptabilise pas le message */
                          spurious++;	/* On comptabilise le Pb */
                          
                          /* Idem pour la comptabilisation par bus */
                          if (bus==1) { nb_msg1--; 
                                        spurious1++;
                                      }
                                 else { nb_msg2--; 
                                        spurious2++; 
                                      }
                          
                          break;
                          
                default : fprintf(stderr,
                                  "type msg 0x%x inconnu (octet # 0x%lx) !\n",
                                  msg_type, octet_courant);
                          exit(-1);
              }
              
       
           for (i = j = 0; i < (nbr_mots - 4 - err - i0); i++, j++)
              { d = lec_mot(1);
                nbmots--;
                if (j == 15) { AJTMP("\n");
                               j = 0;
                             }
                sprintf(temporaire, " %04x", d);
                MAJTMP;
              }
            if (nbr_mots - 4 - err - i0) 
              { AJTMP("\n");
              }
            
           if (err)
             { d = lec_mot(1);
               nbmots--;
               sprintf(temporaire, "ERREUR :   Code = %d [0x%04X]", d, d);
               MAJTMP;
               nb_err++;
               if (bus==1) neb = ++nb_err1; 
                      else neb = ++nb_err2;
               sprintf(temporaire, 
                       "\tErr#%d   Err_Bus%d#%d\n", nb_err, bus, neb);
               MAJTMP;
             }
             
            if (fok)
              { if (err || doinatt)
                  { if (affderok)
                      { vidage_avant_dernier();
                      }
                     vidage_dernier(); 
                  }
                else
                  { bascule();
                  }
              }
            else
              { vidage_dernier();
              }
           }


          }
          
       /* Lecture du numero du tampon ABI */
       i = lec_mot(1);
       if (visutamp) 
         { if (affderok) vidage_avant_dernier();
           printf("------------ Fin du tampon ABI-V3 #%d\n", i);
         }
         
       /* Controle de la succesion des tampons */
       if (vnumtamp)
         { if (i != numero_tampon)
             { fprintf(stderr, "*** Tampon(s) ABI perdu(s) ! ***\n");
               if (affderok) vidage_avant_dernier();
               printf("\n*************************************************\n");
               printf("***   Au moins un tampon ABI a ete perdu :\n");
               printf("***   \tTampon ABI-V3 numero %d a ete lu\n", i);
               printf("***   \tTampon ABI-V3 numero %d etait attendu\n", 
                       numero_tampon);
               printf("*************************************************\n\n");
             }
         }
         
  fprintf(stderr, "numtamp=%d   i=%d\n", numero_tampon, i);
         
       if (i != numero_tampon) numero_tampon = i;
       numero_tampon++;
                 
}   

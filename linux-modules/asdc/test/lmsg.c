/************************************************************************
 * File             : lmsg.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * History
 *
 * Edition Date     Comment                                        By
 * ------- -------- ---------------------------------------------- ---
 *       1  6/06/05 created                                        yg
 *       2 15/02/08 Mise en conformite gcc v4                      yg
 *
 */

/* Lecture et affichage des messages de debug memorises par le driver */

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "asdcctl.h"


 
 
 
main(int argc, char **argv)
{

  int poignee;   
  int i, a, n;
  char *ch;
  
  char *device;
  
  struct asdcmsg msg;
  char zone[TTMSG];
  
  
  if (argc != 2) goto syntaxe;
  
  device = argv[1];    

  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 
    
  msg.ch = zone;

  /* Lecture du tampon des messages */
  if (ioctl(poignee, ASDCLMSG, &msg) == -1 )
    {
      perror("ioctl(ASDCLMSG) ");
      exit(-1);
    }
    
   
  /* Affichage des messages */
  i = 0;
  for (;;)
    {
      switch(msg.ch[i])
        {
          case 0 : // Fin des messages
             printf("---   Fin des messages   ---\n");
             exit(0);
             
          case 1 : // Chaine de caracteres 
             i++;
             ch = &msg.ch[i];
             i += strlen(ch) + 1;
             printf("%s", ch);
             break;
             
          case 2 : // Valeure entiere decimale
             i++;
             a = *((int *) &msg.ch[i]);
             i += 4;
             printf("%d", a);
             break;
          
          case 3 : // Valeure entiere hexadecimale
             i++;
             a = *((int *) &msg.ch[i]);
             i += 4;
             printf("0x%X", a);
             break;
          
          case 4 : // Fin du message
             i++;
             printf("\n");
             break;
             
          default :
             printf("Indice=%d : Code %d inattendu !\n", i, msg.ch[i]);
             exit(-1);        
        }
    
    } 
 
  
  
syntaxe : 
  fprintf(stderr, "Syntaxe : %s device\n", argv[0]);
  exit(-1);

}








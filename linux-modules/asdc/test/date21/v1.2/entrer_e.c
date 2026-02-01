   /**************************************************************/
   /*      Introduction par l'operateur d'une valeur entiere     */
   /*                                                            */
   /*          question : chaine affichee sur stdout pour        */
   /*                     interroger l'operateur                 */
   /*          affdef : format d'affichage de la valeur par      */
   /*                   defaut a inserer dans la question.       */
   /*                   Si affdef est NULL,aucune valeur par     */
   /*                   defaut n'est prevue.                     */
   /*          vmini : Valeur minimale autorisee comme reponse   */
   /*          vmaxi : Valeur maximale autorisee comme reponse   */
   /*          vdef : Pointe une valeur par defaut utilisee si   */
   /*                 pas de reponse                             */
   /*                                                            */
   /*            --> Si vmini >= vmaxi, tout entier est autorise */
   /*                                                            */
   /*          La question est posee a l'operateur jusqu'a       */
   /*          l'obtention d'une reponse correcte (syntaxe et    */
   /*          bornes)                                           */
   /*                                                            */
   /*          La fonction renvoie la valeur fournie par         */
   /*          l'operateur                                       */
   /*                                                            */
   /*                           Anonymized, le 12 mars 1992    */
   /*             introduction val. defaut le 26 fevrier 1998    */
   /*                         derniere modif. le                 */
   /**************************************************************/
   
   
/*
** Utilisation des parametres question et affdef :
** ===============================================
**
**   Le parametre question est une chaine de caracteres qui sera affichee
**   pour demander a l'operateur d'entrer une reponse. En l'absence de
**   valeur par defaut prevue (affdef==NULL), une chaine quelconque fait
**   l'affaire :
**
**              exemple :   "Valeur choisie ? = "
**
**
**   Si une valeur par defaut est donnee, la chaine question doit contenir
**   le symbole %s a l'emplacement voulu pour afficher la valeur par defaut
**   courante.
**   D'autre part, la chaine affdef doit contenir le format utilise pour
**   afficher cette valeur par defaut a l'emplacement indique par le symbole
**   %s dans la chaine question.
**
**              exemple :    si question = "Valeur choisie %s? = "
**                              affdef   = "[%ld] "
**                              vdef = 421
**
**                       alors, la question affichee la premiere fois sera :
**                              "Valeur choisie [421] ? = " 
**
**                       si l'operateur repond par un retour chariot, la
**                       valeur 421 sera prise comme reponse
**
**                       si l'operateur repond par une valeur numerique
**                       autorisee, cette derniere sera prise comme reponse
**
**                       si l'operateur repond par une valeur numerique 
**                       interdite ou par une valeur non numerique, la
**                       question est de nouveau posee, mais sous la forme :
**                               "Valeur choisie ? = "
**                       et il n'y a plus de valeur par defaut.
**
**
**
**  REMARQUE : Si une valeur par defaut est donnee, cette derniere doit 
**             respecter les bornes prevues.
**
*/

#include <stdio.h>
#include <string.h>

#define TAILLE_TAMPON 100
#define TAILLE_QUESTION 100


long int entrer_e(question, affdef, vmini, vmaxi, vdef)
char *question;
char *affdef;
long int vmini, vmaxi;
long int vdef;
{  
   char tampon[TAILLE_TAMPON];
   char tampon1[TAILLE_TAMPON];
   char tampon2[TAILLE_TAMPON];
   char question1[TAILLE_QUESTION];
   char question2[TAILLE_QUESTION];
   char memodef[TAILLE_QUESTION];
   int r, i, n, premiere_fois, vide;
   long int x;
   char *p;
   
   
   /* Elaboration des questions posees */
   if (question == NULL)
     { question1[0] = question2[0] = '\0';
     }
   else
     { if (affdef == NULL)
         { sprintf(question1, question, "");
         }
       else 
         { sprintf(memodef, affdef, vdef);
           sprintf(question1, question, memodef); 
         }
       sprintf(question2, question, "");
     }
   
   
   premiere_fois = 1;
   for (;;)
     { 
       /* Affichage de la question */
       fprintf(stdout, "%s", premiere_fois ? question1 : question2);
       fflush(stdout);
       
       /* Lecture de la reponse */
       fgets(tampon, TAILLE_TAMPON, stdin);
       
       /* Separation en champs (suppression des blancs */
       n = sscanf(tampon, "%s%s", tampon1, tampon2);
       
       switch (n)
          { case 1 : /* Un seul champ : c'est le cas nominal */
                     x = strtol(tampon1, &p, 10);
                     
                     /* Si ok, p pointe sur fin de la chaine */
                     if (*p == '\0') break;
                     
                     /* Sinon, erreur : traitement idem "plusieurs champs" */
                     
            case 2 : /* Plusieurs champs ... */
                     fprintf(stdout , "\007Ceci n'est pas un entier !\n");
                     premiere_fois = 0;
                     continue;
                     
            default : /* Pas de reponse ==> valeur par defaut eventuelle */
                      if (premiere_fois && (affdef != NULL))
                        { x = vdef;
                          break;
                        }
                      else
                        { premiere_fois = 0;
                          continue;
                        }
          }
          
       
       /* Comparaison valeur obtenue aux bornes */          
       if (vmini<vmaxi)
          { if ((vmini>x) || (vmaxi<x))
               { fprintf
                  (stdout , 
                   "\007Cette valeur doit etre comprise entre %ld et %ld !\n",
                   vmini, vmaxi
                  );
                 premiere_fois = 0;
                 continue;
               }
          }
          

       /* La valeur obtenue est satisfaisante ! */ 
       return x;
     } 
    
}                 
                  
   




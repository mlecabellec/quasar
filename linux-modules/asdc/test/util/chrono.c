#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>


static int chronos, chronom;

void inichrono()
{   struct timeb t;

    if(ftime(&t) == -1) { printf(stderr, "initchrono : ERREUR ftime() !!!\n");
                          exit(-1);
                        }
                        
    chronos = t.time;
    chronom = t.millitm;
}

void chrono(seconde, milli)
int *seconde, *milli;
{   struct timeb t;

    if(ftime(&t) == -1) { printf("ERREUR ftime() !!!\n");
                          exit(-1);
                        }
                        
    *seconde = t.time - chronos;
    *milli = t.millitm - chronom;
    
    if (*milli<0) { *milli += 1000;
                    (*seconde)--;
                  }
}


#include <stdio.h>

extern char *tgetstr(), *tgoto(), *getenv();
extern char *calloc();

static char tbuf[512];

char *ptr;

#define TRAITE(mnemo) 						\
        if(!(mnemo = tgetstr("mnemo",&tbufptr))) mnemo = nul;	\
                                            else maj(mnemo);	\
        printf("mnemo = "); 					\
        if(mnemo) prichaine(mnemo);				\
        printf("\n");
        
#define DCL_TRAITE(mnemo)	static char * mnemo;

DCL_TRAITE(DO)	
DCL_TRAITE(xdo)	
DCL_TRAITE(LE)	
DCL_TRAITE(le)	
DCL_TRAITE(UP)	
DCL_TRAITE(up)	
DCL_TRAITE(RI)
DCL_TRAITE(ri)
DCL_TRAITE(ch)	
DCL_TRAITE(cv)	
DCL_TRAITE(cm)
DCL_TRAITE(ho)	
DCL_TRAITE(cl)
DCL_TRAITE(is)	
DCL_TRAITE(ti)	
DCL_TRAITE(te)	
DCL_TRAITE(me)	
DCL_TRAITE(mb)	
DCL_TRAITE(md)	
DCL_TRAITE(mr)	
DCL_TRAITE(sa)	
DCL_TRAITE(ve)	
DCL_TRAITE(vi)	
DCL_TRAITE(vs)	
static int CO, LI;
	
	
	
startup()
{
	register char *term;
	register char *tptr;
	char *tbufptr, *nul;

	tptr = calloc(1024, sizeof(char));
	tbufptr = tbuf;
	
	nul = tbufptr;
	*nul = '\0';
	tbufptr++;

	if(!(term = getenv("TERM")))
		{ printf("Can't get TERM.\n");
		  exit(-1);
		}  
           else printf("TERM = %s\n\n", term);
           
	if(tgetent(tptr,term) < 1)
		{ printf("Terminal %s inconnu !\n", term);
		  exit(-1);
		} 
		
	TRAITE(DO)	
	/* TRAITE(do) : remplace par les 3 lignes ci-dessous */	
        if(!(xdo = tgetstr("do",&tbufptr))) xdo = nul;	
                                       else maj(xdo);	
        printf("do = "); if(xdo) prichaine(xdo); printf("\n");
	TRAITE(LE)	
	TRAITE(le)	
	TRAITE(UP)	
	TRAITE(up)	
	TRAITE(RI)
	TRAITE(ri)
	printf("\n");
		
	TRAITE(ch)	
	TRAITE(cv)	
	TRAITE(cm)
	printf("\n");
		
	TRAITE(ho)	
	TRAITE(cl)
	printf("\n");
		
	TRAITE(is)	
	TRAITE(ti)	
	TRAITE(te)	
	printf("\n");
		
	TRAITE(me)	
	TRAITE(mb)	
	TRAITE(md)	
	TRAITE(mr)	
	printf("\n");
		
	TRAITE(sa)	
	printf("\n");
		
	TRAITE(ve)	
	TRAITE(vi)	
	TRAITE(vs)	
	printf("\n");
		
	CO = tgetnum("co");
	LI = tgetnum("li");
	
        printf("\nterm = %s\n", term);
}


prichaine(ch)
char *ch;
{ int i, l;
  l = strlen(ch);
  for (i=0; i<l; i++)
     { if ((ch[i]>31) && (ch[i]<127) && (ch[i]>=0)) printf("%c",ch[i]);
                                               else printf("<%d>", ch[i]&0xFF);
     }
}                                        
  
out(c)
char c;
{ *(ptr++) = c;
}

maj(seq)
char * seq;
{ char tampon[50];
  ptr = tampon;
  tputs(seq, 1, out);
  *ptr = '\0';
  strcpy(seq, tampon);
}


main()
{    char c;

     startup();
          
     printf("\nLI = %d\tCO = %d\n", LI, CO);
} 

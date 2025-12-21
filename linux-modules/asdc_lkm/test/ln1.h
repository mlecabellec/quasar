/************************************************************************
 * File             : ln1.h
 * Operating System : LynxOS/PowerPC V3.1
 *
 * History
 *
 * Edition Date     Comment                                        By
 * ------- -------- ---------------------------------------------- ---
 *       1  2/04/01 created                                        yg
 *
 *       2 26/09/02 - Passage en unsigned retour LL_get_ram        yg
 *
 *
 * Prototypes des fonctions definies dans ln1.c
 * et macros d'acces ...
 *
 *====================================================================*/


short *LL_init_shared_memory(int poignee);

unsigned short LL_get_ram(int poignee, long offset);
void LL_put_ram(int poignee, long offset, short value);

unsigned short LL_lire_16(int poignee, long offset);
void LL_ecrire_16(int poignee, long offset, short value);

int LL_lire_32(int poignee, long offset);
void LL_ecrire_32(int poignee, long offset, int value);

long LL_get_image(int poignee, long offset);
void LL_put_image(int poignee, long offset, long value);


/* Macros pour acceder a la memoire d'echange du coupleur */
#define LRAM(poignee, adresse)		LL_get_ram(poignee, adresse)
#define ERAM(poignee, adresse, valeur)	LL_put_ram(poignee, adresse, valeur)

/* Macros pour acceder a la memoire "image" du coupleur */
#define LIMA(poignee, adresse)		LL_get_image(poignee, adresse)
#define EIMA(poignee, adresse, valeur)	LL_put_image(poignee, adresse, valeur)

#define LIMAL(poignee, adresse)		LL_get_image_l(poignee, adresse)
#define EIMAL(poignee, adresse, valeur)	LL_put_image_l(poignee, adresse, valeur)
#define LIMAH(poignee, adresse)		LL_get_image_h(poignee, adresse)
#define EIMAH(poignee, adresse, valeur)	LL_put_image_h(poignee, adresse, valeur)


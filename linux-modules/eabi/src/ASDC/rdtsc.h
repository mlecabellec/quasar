
/***************************************************************/
/* Fonctions d'acces sous GNU/Linux a l'horloge TSC du pentium */
/*                                                             */
/*    rdtsc() effectue une lecture brute de l'heure courante   */
/*    (le resultat est exprime en "cycles horloge processeur") */
/*                                                             */
/*    cpufreqhz() donne la frequence du processeur (lue dans   */
/*    /proc/cpuinfo) qui permet de ramener a une grandeur      */
/*    standard la valeur lue par rdtsc()                       */
/*                                                             */
/*                               Anonymized, le 26/11/2004   */
/*                                                             */
/*

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
26/11/2004  YG   Version initiale (systemes 32 bits seulement).
11/04/2013  YG   Adaptation aux systemes 32 et 64 bits

****************************************************************/


#ifndef __EMUABI_RDTSC__
#define __EMUABI_RDTSC__


/// Les mots clefs "static" et "inline" ont de legers effets sur le temps
/// d'execution, mais leur interet ou non reste difficile a determiner...



#ifdef __LP64__

// Version 64 bits
static inline long long liretsc(void)
{
   register unsigned hi, lo;
   asm volatile ("rdtsc" : "=a"(lo), "=d"(hi));
   return ((unsigned long long) lo) | (((unsigned long long) hi) << 32);
}


// Autre version 64 bits
// static inline long long rdtsc(void)
// {
//    register union { unsigned long long lw;
//                     struct { unsigned l; unsigned h; } s;
//    } u;
//    asm volatile ("rdtsc" : "=a"(u.s.l), "=d"(u.s.h));
//    return (u.lw);
// }


#else     /* __LP64__ */


// Version 32 bits
static inline long long liretsc(void)
{
  unsigned long long x;
  __asm__ volatile ("rdtsc" : "=A"(x));
  return x;
}


// Autre version 32 bits
// static inline long long rdtsc(void)
// {
//    register long long TSC asm("eax");
//    asm volatile (".byte 15, 49" : : : "eax", "edx");
//    return TSC;
// }



#endif     /* __LP64__ */


#endif               /* __EMUABI_RDTSC__ */


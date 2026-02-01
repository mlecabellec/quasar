/*============================================================================*
 * FILE:                      C E I _ T Y P E S . H 
 *============================================================================*
 *
 *      COPYRIGHT (C) 1998-2016 BY ABACO SYSTEMS, INC. 
 *      ALL RIGHTS RESERVED.
 *
 *      THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND
 *      COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND WITH
 *      THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR ANY
 *      OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
 *      AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
 *      SOFTWARE IS HEREBY TRANSFERRED.
 *
 *      THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
 *      NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY ABACO SYSTEMS.
 *
 *===========================================================================*
 *
 * FUNCTION:    This file contains platform-specific definitions of common
 *              data types.
 *
 * HISTORY:
 *
 *   Date     By   Vers                        Comments
 * --------  ----  ----  -----------------------------------------------------
 * 03/20/06  skb   1.00  Initial version.
 * 03/03/08  rhc   1.10  Expand 64-bit support.
 * 03/18/08  bdw   1.20  Added new types CEI_NATIVE_INT, CEI_NATIVE_UINT, 
 *                       CEI_NATIVE_LONG, and CEI_NATIVE_ULONG.  These types 
 *                       resolve to native int, unsigned int, long, and 
 *                       unsigned long types, respectively.
 * 03/13/09  skb   1.30  Removed AR15VPX section.  Added CEI_LONG and 
 *                       CEI_ULONG types which resolve to CEI_NATIVE_LONG 
 *                       and CEI_NATIVE_ULONG, respectively.  Removed ARINC
 *                       and CEI_FLOAT types.  Updated CEI_INT32 and 
 *                       CEI_UINT32 types to resolve to int and unsigned int,
 *                       respectively, in Integrity and VxWorks sections.
 * 07/02/10  skb   1.40  Removed '&& !defined(_CVI_)' from the Win32 
 *                       environment block condition.  To exclude definition
 *                       of 64-bit types when compiling under a legacy version
 *                       of CVI that lacks 64-bit support, define the constant
 *                       CEI_TYPES_FORCE_NO_64_BIT.
 * 06/17/11  skb   1.50  Added new convenience pointer types PCEI_<type> which
 *                       are equivalent to the existing pCEI_<type> types.
 * 05/17/16  skb   1.60  Updated copyright info.
 * 09/30/16  bch   1.70  Added support for Integrity x86.
 *
 *===========================================================================*/
#ifndef CEI_TYPES_H
#define CEI_TYPES_H


/* define standard 8-bit, 16-bit, and void types */
typedef char CEI_CHAR;
typedef unsigned char CEI_UCHAR;
typedef short CEI_INT16;
typedef unsigned short CEI_UINT16;
typedef void CEI_VOID;


/* indicate support for 64-bit types - undefined later on if unavailable */
#define CEI_TYPES_64_BIT_AVAIL


/* define standard 32-bit and 64-bit types for known environments */
#ifdef CEI_TYPES_FORCE_NO_64_BIT

   /* force compile without 64-bit support */
   #undef CEI_TYPES_64_BIT_AVAIL

#elif defined(LP64) || defined(_LP64) || defined(__LP64__)

   /* define standard 32-bit and 64-bit types */
   typedef int CEI_INT32;
   typedef unsigned int CEI_UINT32;
   typedef long CEI_INT64;
   typedef unsigned long CEI_UINT64;

#elif defined(_WIN32) || defined(WIN32)

   /* define standard 32-bit and 64-bit types */
   typedef long CEI_INT32;
   typedef unsigned long CEI_UINT32;
   typedef __int64 CEI_INT64;
   typedef unsigned __int64 CEI_UINT64;

#elif defined(_QNXNTO_PCI_X86_) || defined (VXWORKS) || defined (VXW_PCI_PPC) || defined (VXW_VME_PPC) || defined (VXW_VME_X86) || defined (VXW_PCI_X86) || defined (_SOLARIS_SPARC_) | defined (_SOLARIS_X86_) || defined (_SOLARIS_FORTE_)

   /* define standard 32-bit and 64-bit types */
   typedef int CEI_INT32;
   typedef unsigned int CEI_UINT32;
   typedef long long CEI_INT64;
   typedef unsigned long long CEI_UINT64;

#elif defined(__GNUC__) && ((defined(linux) || defined(__linux) || defined(__linux__) || defined(__Lynx__)) && !defined(__STRICT_ANSI__))

   /* define standard 32-bit and 64-bit types */
   typedef int CEI_INT32;
   typedef unsigned int CEI_UINT32;
   typedef long long CEI_INT64; 
   typedef unsigned long long CEI_UINT64;

#elif defined  (INTEGRITY_PCI_X86) || defined (INTEGRITY_PCI_PPC) || defined (INTEGRITY_VME_PPC)

   /* define standard 32-bit and 64-bit types */
   typedef int CEI_INT32;
   typedef unsigned int CEI_UINT32;
   typedef long long CEI_INT64; 
   typedef unsigned long long CEI_UINT64;

#else

   /* no 64-bit support for this environment */
   #undef CEI_TYPES_64_BIT_AVAIL

#endif 


/* check if 64-bit support is available */
#ifdef CEI_TYPES_64_BIT_AVAIL

   /* define 64-bit pointer types */
   typedef CEI_INT64 *pCEI_INT64, *PCEI_INT64;
   typedef CEI_UINT64 *pCEI_UINT64, *PCEI_UINT64;

#else

   /* define standard 32-bit types */
   typedef long CEI_INT32;
   typedef unsigned long CEI_UINT32;

#endif


/* define standard pointer types */
typedef CEI_CHAR *pCEI_CHAR, *PCEI_CHAR;
typedef CEI_UCHAR *pCEI_UCHAR, *PCEI_UCHAR;
typedef CEI_INT16 *pCEI_INT16, *PCEI_INT16;
typedef CEI_UINT16 *pCEI_UINT16, *PCEI_UINT16;
typedef CEI_INT32 *pCEI_INT32, *PCEI_INT32;
typedef CEI_UINT32 *pCEI_UINT32, *PCEI_UINT32;
typedef CEI_VOID *pCEI_VOID, *PCEI_VOID;


/* define native int and long types */
typedef int CEI_NATIVE_INT;
typedef unsigned int CEI_NATIVE_UINT;
typedef long CEI_NATIVE_LONG;
typedef unsigned long CEI_NATIVE_ULONG;

/* define standard types for convenience */
typedef CEI_NATIVE_INT   CEI_INT;
typedef CEI_NATIVE_UINT  CEI_UINT;
typedef CEI_NATIVE_LONG  CEI_LONG;
typedef CEI_NATIVE_ULONG CEI_ULONG;


#endif /* CEI_TYPES_H */

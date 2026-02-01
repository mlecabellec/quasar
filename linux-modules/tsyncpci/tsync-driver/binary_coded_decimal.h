#ifndef __binary_coded_decimal_h__
#define __binary_coded_decimal_h__ 1

#include "tsync_driver_helpers.h"

/* We want to accommodate several different binary-coded decimal
 * formats.  One is to have two nibbles in each 8-bit or 16-bit
 * word. The bad news there is that some of the values put the least
 * significant nibble in the high nibble of its word, and others in
 * the low.  We'll offer separate functions because the code would be
 * a bit convoluted otherwise. 
 * 
 * X Macros are used in lieu of C++ templates to get us the right
 * functionality for three different pointer types.  Note that this
 * will create uintFromBCDArray* and uintFromBCDArray*HighLSN.  The
 * HighLSN variant expects the least-significant nibble to be the high
 * nibble of its byte. */

#define FROM_BCD_FUNCTION_LIST                               \
FROM_BCDFUNC(uint8_t, uintFromBCDArray8)                     \
FROM_BCDFUNC(uint16_t, uintFromBCDArray16)                   \
FROM_BCDFUNC(uint32_t, uintFromBCDArray32)                   \
FROM_BCDFUNC(volatile uint8_t, uintFromVolatileBCDArray8)    \
FROM_BCDFUNC(volatile uint16_t, uintFromVolatileBCDArray16)  \
FROM_BCDFUNC(volatile uint32_t, uintFromVolatileBCDArray32)

#define FROM_BCDFUNC(argType, funcName)                                 \
unsigned int funcName(argType *bcd, unsigned int nibbles);              \
unsigned int funcName##HighLSN(argType *bcd, unsigned int nibbles);

FROM_BCD_FUNCTION_LIST
#undef FROM_BCDFUNC

/* argument bcd should already be in system byte order. */
unsigned int uintFromBCD(unsigned int bcd, unsigned int nibbles);

unsigned char nibblePerByteFromUint(unsigned int asInteger, 
                                    unsigned char *asBytes,
                                    unsigned int nibbles);

unsigned int bcdFromUint(unsigned int asInteger);

#endif

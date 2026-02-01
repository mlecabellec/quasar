#include "binary_coded_decimal.h"

#define FROM_BCDFUNC(ARG_T, FUNC_NAME)                               \
unsigned int FUNC_NAME(ARG_T *bcd, unsigned int nibbles) {      \
    unsigned int exponent;                                      \
                                                                \
    ARG_T *bcdPos = &bcd[(nibbles-1)/2];                        \
    uint32_t multiplier = 1;                                    \
    uint32_t result = 0;                                        \
                                                                \
    for (exponent = 0; exponent < nibbles; exponent++) {        \
        unsigned char decimalDigit;                             \
        if (exponent & 0x1) {                                   \
            decimalDigit = (*bcdPos >> 4) & 0xf;                \
            bcdPos--;                                           \
        }                                                       \
        else {                                                  \
            decimalDigit = (*bcdPos) & 0xf;                     \
        }                                                       \
                                                                \
        result += decimalDigit * multiplier;                    \
        multiplier *= 10;                                       \
    }                                                           \
                                                                \
    return result;                                              \
}

FROM_BCD_FUNCTION_LIST
#undef FROM_BCDFUNC

#define FROM_BCDFUNC(ARG_T, FUNC_NAME)                                       \
unsigned int FUNC_NAME##HighLSN(ARG_T *bcd, unsigned int nibbles) {     \
    unsigned int exponent;                                              \
                                                                        \
    ARG_T *bcdPos = &bcd[nibbles/2];                                    \
    uint32_t multiplier = 1;                                            \
    uint32_t result = 0;                                                \
                                                                        \
    for (exponent = 0; exponent < nibbles; exponent++) {                \
        unsigned char decimalDigit;                                     \
        if (exponent & 0x1) {                                           \
            decimalDigit = (*bcdPos) & 0xf;                             \
        }                                                               \
        else {                                                          \
            decimalDigit = (*bcdPos >> 4) & 0xf;                        \
            bcdPos--;                                                   \
        }                                                               \
                                                                        \
        result += decimalDigit * multiplier;                            \
        multiplier *= 10;                                               \
    }                                                                   \
                                                                        \
    return result;                                                      \
}

FROM_BCD_FUNCTION_LIST
#undef FROM_BCDFUNC

unsigned int uintFromBCD(unsigned int bcd, unsigned int nibbles) {
    unsigned int multiplier = 1;
    unsigned int exponent = 0;
    unsigned int result = 0;

    unsigned int shiftedBCD = bcd;
    unsigned int mask = 0xf;

    for (exponent = 0; exponent < nibbles; exponent++) {
        result += (shiftedBCD & mask) * multiplier;
        shiftedBCD >>= 4;
        multiplier *= 10;
    }
    return result;
}

unsigned char nibblePerByteFromUint(unsigned int asInteger, 
                                    unsigned char *asBytes,
                                    unsigned int nibbles) {
    unsigned char *bytePos = &asBytes[nibbles-1];

    unsigned int temp = asInteger;

    if (bytePos == NULL) {
        return 1;
    }

    do {
        *bytePos = temp % 10;
        temp /= 10;

        bytePos--;
    } while (bytePos >= asBytes);

    return 0;
}

unsigned int bcdFromUint(unsigned int asInteger) {
    unsigned int result = 0;
    unsigned int shift = 0;
    unsigned int remainder = asInteger;

    while (remainder > 0) {
        unsigned int leastSigDigit = remainder % 10;
        result |= leastSigDigit << shift;

        shift += 4;
        remainder /= 10;
    }

    return result;
}
                         

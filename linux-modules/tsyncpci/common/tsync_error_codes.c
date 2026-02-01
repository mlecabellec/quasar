
#include "tsync_error_codes.h"

const char * tsync_strerror(TSYNC_ERROR err) {
    const char * result = "tsync_strerror switch statement failed.";

    switch(err)
    {
    
    /* WARNING: X Macros! */
    /*#define TSYNC_X(name, value, class, string)             \
    case name:                                  \
        result = string " (" #name ")" ;        \
        break;     */           
    #define TSYNC_X(name, value, class, string)             \
    case name:                                  \
        result = string " (" #name ")" ;        \
        break;
                                     \
    TSYNC_ALL_ERROR_FIELDS
    #undef TSYNC_X

    default:
        result = "unrecognized error code!";
    };
    
    return result;
}

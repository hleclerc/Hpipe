#include "Assert.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

namespace Hpipe {

void abort_or_throw() {
    #ifdef TESTING
    throw Abort();
    #else
    abort();
    #endif
}

bool __disp_and_abort_if_not_cond__( bool cond, const char *txt, ... ) {
    if ( not cond ) {
        va_list argp;
        va_start( argp, txt );
        vfprintf( stderr, txt, argp );
        va_end( argp );
        abort_or_throw();
    }
    return true;
}

bool __disp_if_not_cond__( bool cond, const char *txt, ... ) {
    if ( not cond ) {
        va_list argp;
        va_start( argp, txt );
        vfprintf( stderr, txt, argp );
        va_end( argp );
        return false;
    }
    return true;
}

void __disp( const char *txt, ... ) {
    va_list argp;
    va_start( argp, txt );
    vfprintf( stderr, txt, argp );
    va_end( argp );
}

bool __do_nothing__() {
    return true;
}


}

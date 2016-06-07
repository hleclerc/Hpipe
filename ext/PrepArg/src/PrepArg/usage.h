/*
 Copyright 2012 Structure Computation  www.structure-computation.com
 Copyright 2012 Hugo Leclerc

 This file is part of PrepArg.

 PrepArg is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 PrepArg is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with PrepArg. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef PREPARG_DO_NOT_WANT_STDIO_H
#include <stdio.h>
#include <math.h>
#endif

// load STRCMP, ... if not done
#include "default_values.h"


static char _preparg_u( char v ) {
    return v == '_' ? '-' : v;
}

static void _preparg_dispu( const char *v ) {
    for( int i = 0; v[ i ]; ++i )
        PREPARG_DISP_C( _preparg_u( v[ i ] ) );
}

static int _preparg_streq_u2m( const char *v, const char *l ) {
    for( int i = 0; ; ++i ) {
        if ( v[ i ] == 0 and l[ i ] == 0 )
            return 1;
        if ( v[ i ] != _preparg_u( l[ i ] ) )
            return 0;
    }
}

static bool _preparg_atoi( int &res, const char *v ) {
    if ( v[ 0 ] == '-' ) {
        bool r = _preparg_atoi( res, v + 1 );
        res = -res;
        return r;
    }
    res = 0;
    for( int i = 0; v[ i ]; ++i ) {
        if ( v[ i ] < '0' or v[ i ] > '9' )
            return false;
        res = 10 * res + ( v[ i ] - '0' );
    }
    return true;
}

static bool _preparg_atof( double &res, const char *v ) {
    if ( v[ 0 ] == '-' ) {
        bool r = _preparg_atof( res, v + 1 );
        res = -res;
        return r;
    }
    res = 0;
    for( int i = 0; v[ i ]; ++i ) {
        if ( v[ i ] < '0' or v[ i ] > '9' ) {
            if ( v[ i ] == '.' ) {
                double d = 0.1;
                for( ++i; v[ i ]; ++i, d /= 10 ) {
                    if ( v[ i ] < '0' or v[ i ] > '9' ) {
                        if ( v[ i ] == 'e' ) {
                            int expo;
                            if ( not  _preparg_atoi( expo, v + i + 1 ) )
                                return false;
                            res *= pow( 10, expo );
                            return true;
                        }
                        return false;
                    }
                    res += d * ( v[ i ] - '0' );
                }
                return true;
            }
            if ( v[ i ] == 'e' ) {
                int expo;
                if ( not  _preparg_atoi( expo, v + i + 1 ) )
                    return false;
                res *= pow( 10, expo );
                return true;
            }
            return false;
        }
        res = 10 * res + ( v[ i ] - '0' );
    }
    return true;
}
static int usage( const char *prg, const char *msg = 0, int ret = 0 ) {
    if ( msg ) {
        PREPARG_DISP_S( msg );
        PREPARG_DISP_S( "\n" );
    }
    PREPARG_DISP_S( "Usage: '" );
    PREPARG_DISP_S( prg );
    PREPARG_DISP_S( "' [options]" );
    
    // EARG
    #define SARG( SHORT, VAR, HELP, DEFAULT_VALUE )
    #define BARG( SHORT, VAR, HELP, DEFAULT_VALUE )
    #define IARG( SHORT, VAR, HELP, DEFAULT_VALUE )
    #define DARG( SHORT, VAR, HELP, DEFAULT_VALUE )
    #define EARG( VAR, HELP ) PREPARG_DISP_S( " [" HELP "]" );
    #define DESCRIPTION( STR )
    #include PREPARG_FILE
    #include "undefs.h"
    
    PREPARG_DISP_S( "\n" );

    // options
    #define SARG( SHORT, VAR, HELP, DEFAULT_VALUE ) \
        PREPARG_DISP_S( "  " ); \
        if ( SHORT ) { \
            PREPARG_DISP_S( "-" ); \
            PREPARG_DISP_C( SHORT ); \
            PREPARG_DISP_S( " or " ); \
        } \
        PREPARG_DISP_S( "--" ); \
        _preparg_dispu( #VAR ); \
        PREPARG_DISP_S( " arg: " ); \
        PREPARG_DISP_S( HELP ); \
        PREPARG_DISP_S( " (default='" ); \
        PREPARG_DISP_S( DEFAULT_VALUE ); \
        PREPARG_DISP_S( "')\n" )

    #define BARG( SHORT, VAR, HELP, DEFAULT_VALUE ) \
        PREPARG_DISP_S( "  " ); \
        if ( SHORT ) { \
            PREPARG_DISP_S( "-" ); \
            PREPARG_DISP_C( SHORT ); \
            PREPARG_DISP_S( " or " ); \
        } \
        PREPARG_DISP_S( "--" ); \
        _preparg_dispu( #VAR ); \
        PREPARG_DISP_S( ": " ); \
        PREPARG_DISP_S( HELP ); \
        PREPARG_DISP_S( "\n" )

    #define IARG( SHORT, VAR, HELP, DEFAULT_VALUE ) \
        PREPARG_DISP_S( "  " ); \
        if ( SHORT ) { \
            PREPARG_DISP_S( "-" ); \
            PREPARG_DISP_C( SHORT ); \
            PREPARG_DISP_S( " or " ); \
        } \
        PREPARG_DISP_S( "--" ); \
        _preparg_dispu( #VAR ); \
        PREPARG_DISP_S( ": " ); \
        PREPARG_DISP_S( HELP ); \
        PREPARG_DISP_S( "\n" )

    #define DARG( SHORT, VAR, HELP, DEFAULT_VALUE ) \
        PREPARG_DISP_S( "  " ); \
        if ( SHORT ) { \
            PREPARG_DISP_S( "-" ); \
            PREPARG_DISP_C( SHORT ); \
            PREPARG_DISP_S( " or " ); \
        } \
        PREPARG_DISP_S( "--" ); \
        _preparg_dispu( #VAR ); \
        PREPARG_DISP_S( ": " ); \
        PREPARG_DISP_S( HELP ); \
        PREPARG_DISP_S( "\n" )

    #define EARG( VAR, HELP )

    #define DESCRIPTION( STR ) \
        PREPARG_DISP_S( STR ); \
        PREPARG_DISP_S( "\n" )

    #include PREPARG_FILE
    #include "undefs.h"

    return ret;
}

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


// load PREPARG_... if not done
#include "default_values.h"

// CODE
for( int _preparg_num_arg = 1; _preparg_num_arg < PREPARG_ARGC; ++_preparg_num_arg ) {
    const char *_preparg_arg = PREPARG_ARGV[ _preparg_num_arg ];
    if ( _preparg_arg[ 0 ] == '-' ) {
        // long arg ?
        if ( _preparg_arg[ 1 ] == '-' ) {
            #define SARG( SHORT, VAR, HELP, DEFAULT_VALUE ) \
                if ( PREPARG_STREQ_U2M( _preparg_arg + 2, #VAR ) ) { \
                    if ( _preparg_num_arg + 1 >= PREPARG_ARGC ) { \
                        PREPARG_DISP_S( "Error: " ); \
                        PREPARG_DISP_C( SHORT ); \
                        PREPARG_DISP_S( " or --" ); \
                        _preparg_dispu( #VAR ); \
                        PREPARG_DISP_S( " must be followed by an argument.\n\n" ); \
                        PREPARG_ERROR( PREPARG_PRG_NAME ); \
                        return __LINE__; \
                    } \
                    PREPARG_SET_S( VAR, PREPARG_ARGV[ ++_preparg_num_arg ] ); \
                    continue; \
                }
            #define BARG( SHORT, VAR, HELP, DEFAULT_VALUE ) \
                if ( PREPARG_STREQ_U2M( _preparg_arg + 2, #VAR ) ) { \
                    PREPARG_SET_B( VAR, not DEFAULT_VALUE ); \
                    continue; \
                }
            #define IARG( SHORT, VAR, HELP, DEFAULT_VALUE ) \
                if ( PREPARG_STREQ_U2M( _preparg_arg + 2, #VAR ) ) { \
                    int tmp; \
                    if ( _preparg_num_arg + 1 >= PREPARG_ARGC or not _preparg_atoi( tmp, PREPARG_ARGV[ ++_preparg_num_arg ] ) ) { \
                        PREPARG_DISP_S( "Error: " ); \
                        PREPARG_DISP_C( SHORT ); \
                        PREPARG_DISP_S( " or --" ); \
                        _preparg_dispu( #VAR ); \
                        PREPARG_DISP_S( " must be followed by an integer.\n\n" ); \
                        PREPARG_ERROR( PREPARG_PRG_NAME ); \
                        return __LINE__; \
                    } \
                    PREPARG_SET_S( VAR, tmp ); \
                    continue; \
                }
            #define DARG( SHORT, VAR, HELP, DEFAULT_VALUE ) \
                if ( PREPARG_STREQ_U2M( _preparg_arg + 2, #VAR ) ) { \
                    double tmp; \
                    if ( _preparg_num_arg + 1 >= PREPARG_ARGC or not _preparg_atof( tmp, PREPARG_ARGV[ ++_preparg_num_arg ] ) ) { \
                        PREPARG_DISP_S( "Error: " ); \
                        PREPARG_DISP_C( SHORT ); \
                        PREPARG_DISP_S( " or --" ); \
                        _preparg_dispu( #VAR ); \
                        PREPARG_DISP_S( " must be followed by an floating point number.\n\n" ); \
                        PREPARG_ERROR( PREPARG_PRG_NAME ); \
                        return __LINE__; \
                    } \
                    PREPARG_SET_S( VAR, tmp ); \
                    continue; \
                }
            #define EARG( VAR, HELP )
            #define DESCRIPTION( TXT )
            #include PREPARG_FILE
            #include "undefs.h"

            if ( PREPARG_STREQ_U2M( _preparg_arg + 2, "help" ) ) {
                PREPARG_USAGE( PREPARG_PRG_NAME );
                return 0;
            }
        }

        // Try flags
        bool has_flag = false;
        for( int _preparg_num_ch = 1; ; ++_preparg_num_ch ) {
            #define SARG( SHORT, VAR, HELP, DEFAULT_VALUE )
            #define BARG( SHORT, VAR, HELP, DEFAULT_VALUE ) \
                if ( SHORT and _preparg_arg[ _preparg_num_ch ] == SHORT ) { \
                    PREPARG_SET_B( VAR, not DEFAULT_VALUE ); \
                    has_flag = true; \
                    continue; \
                }
            #define IARG( SHORT, VAR, HELP, DEFAULT_VALUE )
            #define DARG( SHORT, VAR, HELP, DEFAULT_VALUE )
            #define EARG( VAR, HELP )
            #define DESCRIPTION( TXT )
            #include PREPARG_FILE
            #include "undefs.h"
            break;
        }
        if ( has_flag )
            continue;

        // arguments that cannot be concatened
        #define SARG( SHORT, VAR, HELP, DEFAULT_VALUE ) \
            if ( _preparg_arg[ 1 ] == SHORT ) { \
                if ( _preparg_arg[ 2 ] == 0 ) { \
                    if ( ++_preparg_num_arg >= PREPARG_ARGC ) { \
                        PREPARG_DISP_S( "Error: " ); \
                        PREPARG_DISP_C( SHORT ); \
                        PREPARG_DISP_S( " or --" ); \
                        _preparg_dispu( #VAR ); \
                        PREPARG_DISP_S( " must be followed by an argument.\n\n" ); \
                        PREPARG_ERROR( PREPARG_PRG_NAME ); \
                        return __LINE__; \
                    } \
                    PREPARG_SET_S( VAR, PREPARG_ARGV[ _preparg_num_arg ] ); \
                } else { \
                    PREPARG_SET_S( VAR, _preparg_arg + 2 ); \
                } \
                continue; \
            }
        #define BARG( SHORT, VAR, HELP, DEFAULT_VALUE )
        #define IARG( SHORT, VAR, HELP, DEFAULT_VALUE ) \
            if ( _preparg_arg[ 1 ] == SHORT ) { \
                int tmp; \
                if ( _preparg_arg[ 2 ] == 0 ? \
                        ++_preparg_num_arg >= PREPARG_ARGC or not _preparg_atoi( tmp, PREPARG_ARGV[ _preparg_num_arg ] ) : \
                        not _preparg_atoi( tmp, _preparg_arg + 2 ) ) { \
                    PREPARG_DISP_S( "Error: " ); \
                    PREPARG_DISP_C( SHORT ); \
                    PREPARG_DISP_S( " or --" ); \
                    _preparg_dispu( #VAR ); \
                    PREPARG_DISP_S( " must be followed by an integer.\n\n" ); \
                    PREPARG_ERROR( PREPARG_PRG_NAME ); \
                    return __LINE__; \
                } \
                PREPARG_SET_I( VAR, tmp ); \
                continue; \
            }
        #define DARG( SHORT, VAR, HELP, DEFAULT_VALUE ) \
            if ( _preparg_arg[ 1 ] == SHORT ) { \
                double tmp; \
                if ( _preparg_arg[ 2 ] == 0 ? \
                        ++_preparg_num_arg >= PREPARG_ARGC or not _preparg_atof( tmp, PREPARG_ARGV[ _preparg_num_arg ] ) : \
                        not _preparg_atof( tmp, _preparg_arg + 2 ) ) { \
                    PREPARG_DISP_S( "Error: " ); \
                    PREPARG_DISP_C( SHORT ); \
                    PREPARG_DISP_S( " or --" ); \
                    _preparg_dispu( #VAR ); \
                    PREPARG_DISP_S( " must be followed by an integer.\n\n" ); \
                    PREPARG_ERROR( PREPARG_PRG_NAME ); \
                    return __LINE__; \
                } \
                PREPARG_SET_I( VAR, tmp ); \
                continue; \
            }
        #define EARG( VAR, HELP )
        #define DESCRIPTION( TXT )
        #include PREPARG_FILE
        #include "undefs.h"

        if ( _preparg_arg[ 1 ] == 'h' ) {
            PREPARG_USAGE( PREPARG_PRG_NAME );
            return 0;
        }
    }

    // else -> ending argument
    #define SARG( SHORT, VAR, HELP, DEFAULT_VALUE )
    #define BARG( SHORT, VAR, HELP, DEFAULT_VALUE )
    #define IARG( SHORT, VAR, HELP, DEFAULT_VALUE )
    #define DARG( SHORT, VAR, HELP, DEFAULT_VALUE )
    #define EARG( VAR, HELP ) PREPARG_SET_I( VAR, _preparg_num_arg ); break
    #define DESCRIPTION( TXT )
    #include PREPARG_FILE
    #include "undefs.h"
}


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


#ifndef DEFAULT_VALUES_H
#define DEFAULT_VALUES_H


#ifndef PREPARG_DISP_S
#define PREPARG_DISP_S( S ) printf( "%s", ( S ) ? ( S ) : "" );
#endif

#ifndef PREPARG_DISP_C
#define PREPARG_DISP_C( S ) printf( "%c", S );
#endif

#ifndef PREPARG_DISP_I
#define PREPARG_DISP_I( S ) printf( "%i", S );
#endif

#ifndef PREPARG_DISP_B
#define PREPARG_DISP_B( S ) PREPARG_DISP_I( S )
#endif

#ifndef PREPARG_S
#define PREPARG_S( A ) const char *A
#endif

#ifndef PREPARG_B
#define PREPARG_B( A ) bool A
#endif

#ifndef PREPARG_I
#define PREPARG_I( A ) int A
#endif

#ifndef PREPARG_D
#define PREPARG_D( A ) double A
#endif

#ifndef PREPARG_S_D
#define PREPARG_S_D( A, DEFAULT_VALUE ) PREPARG_S( A ) = DEFAULT_VALUE
#endif

#ifndef PREPARG_B_D
#define PREPARG_B_D( A, DEFAULT_VALUE ) PREPARG_B( A ) = DEFAULT_VALUE
#endif

#ifndef PREPARG_I_D
#define PREPARG_I_D( A, DEFAULT_VALUE ) PREPARG_I( A ) = DEFAULT_VALUE
#endif

#ifndef PREPARG_D_D
#define PREPARG_D_D( A, DEFAULT_VALUE ) PREPARG_D( A ) = DEFAULT_VALUE
#endif

#ifndef PREPARG_ARGC
#define PREPARG_ARGC argc
#endif

#ifndef PREPARG_ARGV
#define PREPARG_ARGV argv
#endif

#ifndef PREPARG_GET_SET_PREFIX
#define PREPARG_GET_SET_PREFIX
#endif

#ifndef PREPARG_SET_S
#define PREPARG_SET_S( VAR, VAL ) PREPARG_GET_SET_PREFIX VAR = VAL;
#endif

#ifndef PREPARG_SET_B
#define PREPARG_SET_B( VAR, VAL ) PREPARG_GET_SET_PREFIX VAR = VAL;
#endif

#ifndef PREPARG_SET_I
#define PREPARG_SET_I( VAR, VAL ) PREPARG_GET_SET_PREFIX VAR = VAL;
#endif

#ifndef PREPARG_STREQ_U2M
#define PREPARG_STREQ_U2M( A, B ) _preparg_streq_u2m( A, B )
#endif

#ifndef PREPARG_USAGE
#define PREPARG_USAGE( P ) usage( P );
#endif

#ifndef PREPARG_ERROR
#define PREPARG_ERROR( P ) PREPARG_DISP_S( "Try '" ); PREPARG_DISP_S( P ); PREPARG_DISP_S( " --help' for more information.\n" );
#endif

#ifndef PREPARG_PRG_NAME
#define PREPARG_PRG_NAME PREPARG_ARGV[ 0 ]
#endif

#endif // DEFAULT_VALUES_H

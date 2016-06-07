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


DESCRIPTION( "test prg" );

SARG( 's', my_string, "A string", "default value" );
BARG( 'b', my_bool  , "A boolean", false );
BARG( 'c', my_anbool, "Another boolean", false );
IARG( 'i', my_int   , "An integer", 0 );

EARG( my_ending_argument, "files" );

/*
 Copyright 2012 Structure Computation  www.structure-computation.com
 Copyright 2012 Hugo Leclerc

 This file is part of Hpipe.

 Hpipe is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hpipe is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with Hpipe. If not, see <http://www.gnu.org/licenses/>.
*/


#include "Hpipe/Stream.h"
#include <fstream>
using namespace std;

int usage( const char *prg, const char *msg, int res ) {
    if ( msg )
        std::cerr << msg << std::endl;
    std::cerr << "Usage:" << std::endl;
    std::cerr << "  " << prg << " output_file input_file cpp_var_name" << std::endl;
    return res;
}

int main( int argc, char **argv ) {
    if ( argc != 4 )
        return usage( argv[ 0 ], "nb args", 1 );
    std::ofstream fo( argv[ 1 ] );
    std::ifstream fi( argv[ 2 ] );
    if ( not fi or not fo )
        return 2;

    fo << "char " << argv[ 3 ] << "[] = {";
    for( int i = 0; ; ++i ) {
        int c = fi.get();
        if ( fi.eof() )
            c = 0;

        if ( i )
            fo << ", ";
        if ( i % 16 == 0 )
            fo << "\n    ";
        fo << c;

        if ( fi.eof() ) {
            if ( i )
                fo << "\n";
            break;
        }
    }
    fo << "};\n";

    return 0;
}

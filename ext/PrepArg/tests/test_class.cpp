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


#define PREPARG_GET_SET_PREFIX my_instance.
#define PREPARG_FILE "../../tests/test_args.h"
#include <PrepArg/usage.h>

struct MyClass {
    MyClass() {
        #include <PrepArg/constructor.h>
    }
    #include <PrepArg/attributes.h>
};

int main( int argc, char **argv ) {
    MyClass my_instance;

    #include <PrepArg/parse.h>
    #include <PrepArg/info.h>
}


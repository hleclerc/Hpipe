/*
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

#pragma once

#include "Stream.h"
#include <sstream>
#include <fstream>

namespace Hpipe {

std::ostream &dot_out( std::ostream &os, const char *beg, int lim = -1 );
std::ostream &dot_out( std::ostream &os, const char *beg, const char *end, int lim = -1 );

template<class T>
std::ostream &dot_out( std::ostream &os, const T &val, int lim = -1 ) {
    std::ostringstream ss;
    ss << val;
    return dot_out( os, ss.str().c_str(), lim );
}


int exec_dot( const char *filename, const char *viewer = 0, bool launch_viewer = true, bool par = true );

} // namespace Hpipe

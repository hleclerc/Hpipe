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

#include "Cond.h"

namespace Hpipe {

/**
*/
class CondWE {
public:
    bool operator<( const CondWE &cwe ) const { return std::tie( cond, eof ) < std::tie( cwe.cond, cwe.eof ); }

    Cond cond;
    bool eof;
};

} // namespace Hpipe

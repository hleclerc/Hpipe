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


#ifndef STREAM_H
#define STREAM_H

#include "StreamSep.h"

namespace Hpipe {

extern StreamSepMaker coutn;
extern StreamSepMaker cerrn;

typedef std::string String; // HUM

#define P( A ) Hpipe::coutn << #A << " -> " << ( A )
#define PE( A ) Hpipe::cerrn << #A << " -> " << ( A )

}

#endif // STREAM_H

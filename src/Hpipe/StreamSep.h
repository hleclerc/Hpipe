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

#include <iostream>
#include <string>

namespace Hpipe {

/**

*/
class StreamSep {
public:
    using TS = std::ostream;

    StreamSep( TS *stream, const std::string &sep, const std::string &end ) : stream( stream ), sep( sep ), end( end ) {}
    ~StreamSep() { *stream << end; }

    ///
    template<class T>
    StreamSep &operator<<( const T &val ) {
        *stream << sep << val;
        return *this;
    }

    TS         *stream;
    std::string sep;
    std::string end;
};


/**

*/
class StreamSepMaker {
public:
    using TS = std::ostream;

    StreamSepMaker( TS &stream, const std::string &beg = {}, const std::string &sep = {}, const std::string &end = "\n", const std::string &first_beg = {} ) : stream( &stream ), first_beg( first_beg ), beg( beg ), sep( sep ), end( end ) {}
    StreamSepMaker( StreamSepMaker &ss, const std::string &add_beg = {} ) : stream( ss.stream ), first_beg( ss.first_beg ), beg( ss.beg + add_beg ), sep( ss.sep ), end( ss.end ) {}

    ///
    template<class T>
    StreamSep operator<<( const T &val ) {
        if( first_beg.size() ) {
            *stream << first_beg;
            first_beg.clear();
        } else
            *stream << beg;
        *stream << val;
        return StreamSep( stream, sep, end );
    }

    ///
    StreamSepMaker rm_beg( unsigned n ) {
        StreamSepMaker res = *this;
        res.beg.resize( beg.size() - std::min( unsigned( beg.size() ), n ) );
        return res;
    }

    TS         *stream;
    std::string first_beg;
    std::string beg;
    std::string sep;
    std::string end;
};

}

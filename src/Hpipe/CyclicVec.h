#pragma once

#include <Vec.h>

namespace Hpipe {

template<class T>
class CyclicVec {
public:
    using S = unsigned;

    CyclicVec( S size ) {
        data.resize( size );
        off = 0;
    }

    const T &operator[]( S index ) { return data[ ( index + off ) % data.size() ]; }

    void operator<<( const T &val ) { data[ ++off % data.size() ] = val; }

    Vec<T> data;
    S      off;
};

}

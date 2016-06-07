#pragma once

#include <functional>

namespace Hpipe {

template<class V,class F>
bool combinatorial_find( const V &v, unsigned n, const F &f, unsigned min = 0, const V &a = {}, const V &b = {} ) {
    if ( not n ) {
        V bn = b;
        for( unsigned i = min; i < v.size(); ++i )
            bn << v[ i ];
        return f( a, bn );
    }
    for( unsigned i = min; i < v.size(); ++i ) {
        // not taken
        V bn = b;
        for( unsigned j = min; j < i; ++j )
            bn << v[ j ];
        // taken
        V an = a;
        an << v[ i ];
        if ( combinatorial_find( v, n - 1, f, i + 1, an, bn ) )
            return true;
    }
    return false;
}


}

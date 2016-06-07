#pragma once

#include <utility>
#include "Print.h"

namespace Hpipe {

template<class T,int s = 32>
class Pool {
public:
    Pool() : last( 0 ), used( s ) {
    }

    ~Pool() {
        if ( last ) {
            // delete the last
            for( int i = used; i--; )
                reinterpret_cast<T *>( last->items + sizeof( T ) * i )->~T();

            // previous items
            Set *c = last->prev;
            delete last;
            while( c ) {
                Set *p = c->prev;
                for( int i = s; i--; )
                    reinterpret_cast<T *>( c->items + sizeof( T ) * i )->~T();
                delete c;
                c = p;
            }
        }
    }



    template<class... Args>
    T *New( Args&&... args ) {
        if ( used == s ) {
            Set *n = new Set;
            n->prev = last;
            last = n;
            used = 0;
        }
        return new ( last->items + sizeof( T ) * used++ ) T( std::forward<Args>( args )... );
    }

private:
    struct Set {
        char items[ s * sizeof( T ) ];
        Set *prev;
    };
    Set *last;
    int  used; ///< in last Set
};

}

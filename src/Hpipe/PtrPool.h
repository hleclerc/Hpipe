#pragma once

#include <utility>

namespace Hpipe {

template<class T,int s = 32>
class PtrPool {
public:
    PtrPool() : last( 0 ), used( s ) {
    }

    ~PtrPool() {
        if ( last ) {
            // delete the last
            for( int i = used; i--; )
                delete last->items[ i ];

            // previous items
            Set *c = last->prev;
            delete last;
            while( c ) {
                Set *p = c->prev;
                for( int i = s; i--; )
                    delete c->items[ i ];
                delete c;
                c = p;
            }
        }
    }

    template<class A>
    A *operator<<( A *ptr ) {
        if ( used == s ) {
            Set *n = new Set;
            n->prev = last;
            last = n;
            used = 0;
        }
        last->items[ used++ ] = ptr;
        return ptr;
    }

private:
    struct Set {
        T   *items[ s ];
        Set *prev;
    };
    Set *last;
    int  used; ///< in last Set
};

}

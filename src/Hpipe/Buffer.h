#pragma once

#include "inc_and_dec_ref.h"
#include <stdlib.h>

namespace Hpipe {

/**
*/
class Buffer {
public:
    enum { default_size = 2048 - 3 * sizeof( unsigned ) - sizeof( Buffer * ) };
    using PI8 = unsigned char;
    using PT  = unsigned;

    static Buffer *New( unsigned size = default_size, Buffer *prev = 0 ) {
        #ifdef HPIPE_CHECK_ALIVE_BUF
        ++nb_alive_bufs;
        #endif // HPIPE_CHECK_ALIVE_BUF
        Buffer *res = (Buffer *)malloc( sizeof( Buffer ) + size - 4 );
        if ( prev ) prev->next = res;
        res->cpt_use = 0;
        res->used    = 0;
        res->next    = 0;
        res->size    = size;
        return res;
    }

    static void operator delete( void *ptr ) {
        #ifdef HPIPE_CHECK_ALIVE_BUF
        --nb_alive_bufs;
        #endif // HPIPE_CHECK_ALIVE_BUF
        free( ptr );
    }

    unsigned room() const {
        return size - used;
    }

    PT cum_size() const {
       PT res = 0;
       for( const Buffer *b = this; b; b = b->next )
           res += b->used;
       return res;
    }

    void dec_ref_rec() {
        for( Buffer *buf = this; buf; ) {
            Hpipe::Buffer *old = buf;
            buf = buf->next;
            dec_ref( old );
        }
    }

    void dec_ref_upto( Buffer *dst ) {
        for( Buffer *buf = this; buf != dst; ) {
            Hpipe::Buffer *old = buf;
            buf = buf->next;
            dec_ref( old );
        }
    }

    // attributes
    mutable int cpt_use;   ///< destroyed if < 0
    unsigned    used;      ///< nb items stored in data
    Buffer     *next;      ///<
    unsigned    size;      ///< real size of data[]
    PI8         data[ 4 ]; ///<
    #ifdef HPIPE_CHECK_ALIVE_BUF
    static int  nb_alive_bufs;
    #endif // HPIPE_CHECK_ALIVE_BUF
};

}

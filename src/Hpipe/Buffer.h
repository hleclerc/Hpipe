#pragma once

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

    static void skip( Buffer *&buf, const PI8 *&data, unsigned nb_to_skip ) {
        while ( nb_to_skip >= buf->data + buf->used - data ) {
            nb_to_skip -= buf->data + buf->used - data;
            Buffer *old = buf;
            buf = buf->next;
            dec_ref( old );
            if ( ! buf )
                return;
            data = buf->data;
        }
        data += nb_to_skip;
    }

    static const Buffer *inc_ref( const Buffer *p ) {
        ++p->cpt_use;
        return p;
    }

    static Buffer *inc_ref( Buffer *p ) {
        ++p->cpt_use;
        return p;
    }

    static void dec_ref( const Buffer *p ) {
        if ( --p->cpt_use < 0 )
            delete p; // delete has been surdefined
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
            Buffer *old = buf;
            buf = buf->next;
            dec_ref( old );
        }
    }

    void dec_ref_upto( Buffer *dst ) {
        for( Buffer *buf = this; buf != dst; ) {
            Buffer *old = buf;
            buf = buf->next;
            dec_ref( old );
        }
    }

    const PI8 *begin() const {
        return data;
    }

    PI8 *begin() {
        return data;
    }

    const PI8 *end() const {
        return data + used;
    }

    PI8 *end() {
        return data + used;
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

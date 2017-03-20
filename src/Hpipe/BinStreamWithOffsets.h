#pragma once

#include "../Containers/EnableIf.h"
#include "BinStream.h"
#include <vector>

namespace Hpipe {

template<class TB=CbQueue,bool only_positive_offsets=true>
struct BinStreamWithOffsets : BinStream<TB> {
    using TO = typename EnableIf<only_positive_offsets+1,ST,PT>::T;

    BinStreamWithOffsets( TB *buf ) : BinStream<TB>( buf ) {
    }

    void crunch() {
        if ( offsets_to_int_to_reduce.size() == 0 )
            return;

        // get needed size for each int to reduce
        if ( only_positive_offsets ) {
            std::vector<ST> sizeof_rems( offsets_to_int_to_reduce.size(), 0 ); // nb bytes to remove at each int_to_reduce position
            for( PT i = offsets_to_int_to_reduce.size(); i--; ) {
                TO  pos = offsets_to_int_to_reduce[ i ];                   // location of the offset data
                #ifdef Hpipe_JS
                PI8 *ptr = (PI8 *)this->buf->ptr( pos );                   // pointer to the offset data
                TO   val;                                                  // offset value
                memcpy( &val, ptr, sizeof( TO ) );
                #else
                TO *ptr = reinterpret_cast<TO *>( this->buf->ptr( pos ) ); // pointer to the offset data
                TO  val = *ptr;                                            // offset value
                #endif
                TO  lim = pos + sizeof( TO ) + val;                        // pointed position

                // compute the new val
                for( PT j = i + 1; j < offsets_to_int_to_reduce.size() and offsets_to_int_to_reduce[ j ] < lim; ++j )
                    val -= sizeof_rems[ j ]; // to update offset value according to needed sizes after pos and before lim
                sizeof_rems[ i ] = sizeof( TO ) - this->size_needed_for( val );

                // write the new val
                CmQueue cm( ptr, ptr + 16 ); BinStream<CmQueue> bw( &cm ); bw << val;
            }

            // update the buffer, with updated offsets
            for( PT i = offsets_to_int_to_reduce.size(); i--; )
                offsets_to_int_to_reduce[ i ] += sizeof( TO ) - sizeof_rems[ i ]; // offset_to_... become a list of offset of data to be removed
            this->buf->remove_chunks( offsets_to_int_to_reduce, sizeof_rems );
            offsets_to_int_to_reduce.clear();
        } else {
            TODO;
        }
    }

    void beg_mark() {
        offsets_to_active_beg_msg_length.push_back( TO( this->size() ) );
        offsets_to_int_to_reduce.push_back( TO( this->size() ) );
        this->buf->make_room( sizeof( TO ) );
    }

    void end_mark() {
        TO o = offsets_to_active_beg_msg_length.back();
        offsets_to_active_beg_msg_length.pop_back();
        #ifdef Hpipe_JS
        TO v = this->buf->size() - o - sizeof( TO );
        memcpy( this->buf->ptr( o ), &v, sizeof( TO ) );
        #else
        *reinterpret_cast<TO *>( this->buf->ptr( o ) ) = this->buf->size() - o - sizeof( TO );
        #endif
    }

    ST nb_open_marks() const {
        return offsets_to_active_beg_msg_length.size();
    }

    std::vector<TO> offsets_to_active_beg_msg_length; ///< stack
    std::vector<TO> offsets_to_int_to_reduce;
};


} // namespace Hpipe


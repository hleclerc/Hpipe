#pragma once

#include "BinStream.h"
#include "EnableIf.h"
#include "Assert.h"
#include <vector>

namespace Hpipe {

template<class TB=CbQueue,bool only_positive_offsets=true>
struct BinStreamWithOffsets : BinStream<TB> {
    using TO = typename EnableIf<only_positive_offsets+1,ST,PT>::T;

    BinStreamWithOffsets( TB *buf ) : BinStream<TB>( buf ) {
    }

    void crunch() {
        if ( offsets.empty() )
            return;

        // get needed size for each int to reduce
        if ( only_positive_offsets ) {
            std::vector<ST> sizeof_rems( offsets.size(), 0 ); // nb bytes to remove at each int_to_reduce position
            for( PT i = offsets.size(); i--; ) {
                TO  pos = offsets[ i ];                                    // location of the offset data
                TO *ptr = reinterpret_cast<TO *>( this->buf->ptr( pos ) ); // pointer to the offset data
                TO  lim = *ptr;                                            // offset value
                TO  off = lim - pos;                                       // offset value

                // compute the new val
                if ( off > 0 ) {
                    for( PT j = i + 1; j < offsets.size() and offsets[ j ] < lim; ++j )
                        off -= sizeof_rems[ j ]; // to update offset value according to needed sizes after pos and before lim
                    for ( TO old_need = sizeof( TO ), base_off = off - old_need; ; ) {
                        TO need = this->size_needed_for( off );
                        if ( old_need == need )
                            break;
                        off = base_off + need;
                        old_need = need;
                    }
                }
                sizeof_rems[ i ] = sizeof( TO ) - this->size_needed_for( off );

                // write the new val
                CmQueue cm( ptr, ptr + 2 ); BinStream<CmQueue> bw( &cm ); bw << off;
            }

            // update the buffer, with updated offsets
            for( size_t i = 0; i < offsets.size(); ++i )
                offsets[ i ] += sizeof( TO ) - sizeof_rems[ i ];
            this->buf->remove_chunks( offsets, sizeof_rems );
            offsets.clear();
        } else {
            HPIPE_TODO;
        }
    }

    ///
    TO *new_offset() {
        offsets.push_back( this->size() );
        TO *res = reinterpret_cast<TO *>( this->buf->make_room( sizeof( TO ) ) );
        *res = 0;
        return res;
    }

    std::vector<TO> offsets;
};


} // namespace Hpipe


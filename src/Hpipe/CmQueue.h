#pragma once

#include "CbQueue.h"

namespace Hpipe {

/**
  To read or write data from reserved room in memory (data is not owned by CmQueue, it has to be reserved and destroyed elsewhere)
*/
class CmQueue {
public:
    CmQueue( void *beg = 0 ) : beg( (PI8 *)beg ), end( (PI8 *)beg ) {}

    // error
    operator bool() const { return not error(); }
    bool error() const { return beg == 0; } ///< works after at least a first read (and before free or clear)
    bool ack_error() { beg = 0; end = 0; return false; } ///< set error flag to true, and return false

    // size
    bool empty() const { return end == beg; }
    ST   size () const { return end - beg; }

    // writers
    void write_some( const void *data, ST size ) { memcpy( end, data, size ); end += size; }
    void write_some( const CbQueue &s ) { s.data_visitor( [ this ]( const PI8 *b, const PI8 *e ) { memcpy( end, b, e - b ); end += e - b; } ); }
    void write_byte( PI8 val ) { *( end++ ) = val; }
    void write_byte_wo_beg_test( PI8 val ) { write_byte( val ); }

    // readers. Beware there are no checks in these methods
    void read_some( void *data, ST size ) { memcpy( data, beg, size ); beg += size; }
    void skip_some( ST size ) { beg += size; }

    PI8  read_byte() { return *( beg++ ); }
    const PI8 *ptr() const { return beg; }

    // checkings for readers that save a signal (that will give error() != 0) if not ok. To be done before each read.
    bool ack_read_byte() { return beg < end ? true : ack_error(); } ///< return true if ok to read a byte. Else, set end to 0 (to signal an error) and return false.
    bool ack_read_some( ST len ) { return beg + len <= end ? true : ack_error(); } ///< return true if ok to `len` bytes. Else, set end to 0 (to signal an error) and return false.

    // display
    void write_to_stream( std::ostream &os ) const { os.write( (const char *)beg, end - beg ); }

    operator std::string() { return { beg, end }; }

protected:
    PI8 *beg;
    PI8 *end;
};

} // namespace Hpipe

#pragma once

#include "CbString.h"
#include "CbQueue.h"
#include <string.h>

namespace Hpipe {

/**
*/
template<class TB>
struct TxtStream {
    TxtStream( TB *buf ) : buf( buf ) {}

    operator bool() const { return not buf->error(); }
    bool error() const { return buf->error(); }
    bool empty() const { return buf->empty(); }
    PT size() const { return buf->size(); }

    // generic writers
    TxtStream &write_some( const void *ptr, ST len ) {
        buf->write_some( ptr, len );
        return *this;
    }

    TxtStream &write_byte( PI8 val ) {
        buf->write_byte( val );
        return *this;
    }

    // integer writers
    template<class T>
    TxtStream &write_unsigned( const T &val ) {
        if ( not val )
            return write_byte( '0' );
        write_unsigned_rec( val );
        return *this;
    }

    template<class T>
    TxtStream &write_signed( const T &val ) {
        if ( val >= 0 )
            return write_unsigned( val );
        write_byte( '-' );
        return write_unsigned( -val );
    }

    // writers
    TxtStream &operator<<( Bool val ) { return write_unsigned( val ); }

    TxtStream &operator<<( PI8  val ) { return write_unsigned( val ); }
    TxtStream &operator<<( PI16 val ) { return write_unsigned( val ); }
    TxtStream &operator<<( PI32 val ) { return write_unsigned( val ); }
    TxtStream &operator<<( PI64 val ) { return write_unsigned( val ); }

    TxtStream &operator<<( SI8  val ) { return write_signed( val ); }
    TxtStream &operator<<( SI16 val ) { return write_signed( val ); }
    TxtStream &operator<<( SI32 val ) { return write_signed( val ); }
    TxtStream &operator<<( SI64 val ) { return write_signed( val ); }

    TxtStream &operator<<( char val ) { return write_byte( val ); }

    TxtStream &operator<<( const char *c_str ) { return write_some( c_str, strlen( c_str ) ); }
    TxtStream &operator<<( const std::string &str ) { return write_some( str.data(), str.size() ); }

    TxtStream &operator<<( TxtStream &&str ) { buf->write_some( std::move( *str.buf ) ); return *this; }


    // readers
    template<class T>
    void read_signed( T &val ) {
        if ( not buf->ack_read_byte() )
            return;
        // first char
        PI8 c = buf->read_byte();
        if ( c == '-' ) {
            read_signed( val );
            val = -val;
            return;
        }
        if ( c < '0' or c > '9' ) {
            buf->ack_error();
            return;
        }
        val = c - '0';

        // continuation
        while ( true ) {
            if ( not buf->ack_read_byte() )
                return;
            PI8 c = buf->read_byte();
            if ( c < '0' or c > '9' )
                return;
            val = 10 * val + ( c - '0' );
        }
    }

    template<class T>
    void read_unsigned( T &val ) {
        // first char
        if ( not buf->ack_read_byte() )
            return;
        PI8 c = *buf->ptr();
        if ( c < '0' or c > '9' ) {
            buf->ack_error();
            return;
        }
        buf->read_byte();
        val = c - '0';

        // continuation
        while ( true ) {
            if ( buf->empty() )
                return;
            PI8 c = *buf->ptr();
            if ( c < '0' or c > '9' )
                return;
            val = 10 * val + ( c - '0' );
            buf->read_byte();
        }
    }

    // read helpers
    void read( PI8  &val ) { read_unsigned( val ); }
    void read( PI16 &val ) { read_unsigned( val ); }
    void read( PI32 &val ) { read_unsigned( val ); }
    void read( PI64 &val ) { read_unsigned( val ); }

    void read( SI8  &val ) { read_signed( val ); }
    void read( SI16 &val ) { read_signed( val ); }
    void read( SI32 &val ) { read_signed( val ); }
    void read( SI64 &val ) { read_signed( val ); }

    void read( CbString &val ) { val = buf->read_line( ' ' ); }

    // std notations
    template<class T>
    TxtStream &operator>>( T &val ) {
        read( val );
        return *this;
    }

    // readers like T res = b.read() (or b.read_unsigned(), ...)
    struct UnsReader { template<class T> operator T() { T res; b->read_unsigned( res ); return res; } TxtStream *b; };
    struct SigReader { template<class T> operator T() { T res; b->read_signed( res ); return res; } TxtStream *b; };
    struct GenReader { template<class T> operator T() { T res; b->read( res ); return res; } TxtStream *b; };

    UnsReader read_unsigned() { return { this }; }
    SigReader read_signed() { return { this }; }
    GenReader read() { return { this }; }

    TB *buf;

protected:
    template<class T>
    void write_unsigned_rec( const T &val ) {
        if ( val ) {
            write_unsigned_rec( val / 10 );
            buf->write_byte( '0' + val % 10 );
        }
    }
};

} // namespace Hpipe

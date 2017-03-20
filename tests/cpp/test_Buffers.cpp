//// nsmake global inc_path ..

#include "../src/Hpipe/BinStreamWithOffsets.h"
#include "../src/Hpipe/CbStringPtr.h"
#include "../src/Hpipe/TxtStream.h"
#include <gtest/gtest.h>

using namespace Hpipe;

TEST( Buffers, bin_stream_cb ) {
    PI32 uvals[] = { 1,  10, 100,  1000 };
    SI32 svals[] = { 1, -10, 100, -1000, - 1024 * 1024, -5 };

    // bin writer
    CbQueue os;
    BinStream<CbQueue> bw( &os );
    for( PI32 u : uvals )
        bw.write_unsigned( u );
    for( SI32 s : svals )
        bw.write_signed( s );

    // copy
    CbString cs( os );
    BinStream<CbString> bs( &cs );

    // bin reader
    auto f = [ uvals, svals ]( auto bw ) {
        for( PI32 u : uvals ) {
            int r = bw.read_unsigned();
            EXPECT_EQ( bw.empty(), 0 );
            EXPECT_EQ( bw.error(), 0 );
            EXPECT_EQ( r, u );
        }
        for( SI32 s : svals ) {
            int r = bw.read_signed();
            EXPECT_EQ( bw.empty(), s == - 5 );
            EXPECT_EQ( bw.error(), 0 );
            EXPECT_EQ( r, s );
        }
        (int)bw.read_signed();
        EXPECT_EQ( bw.error(), true );
        EXPECT_EQ( bw.empty(), true );
        EXPECT_EQ( bw.size(), 0 );
    };

    f( bw );
    f( bs );
}

TEST( Buffers, txt_stream_cb ) {
    // txt writer
    CbQueue os;
    TxtStream<CbQueue> bw( &os );

    bw << 127;
    EXPECT_EQ( std::string( os ), "127" );
    EXPECT_EQ( int( bw.read_unsigned() ), 127 );
}


TEST( Buffers, bin_stream_cm ) {
    PI32 uvals[] = { 1,  10, 100,  1000 };
    SI32 svals[] = { 1, -10, 100, -1000, - 1024 * 1024, -5 };
    PI8  rdata[ 28 ];

    // bin writer
    CmQueue os( rdata, rdata + sizeof rdata );
    BinStream<CmQueue> bw( &os );
    for( PI32 u : uvals )
        bw.write_unsigned( u );
    for( SI32 s : svals )
        bw.write_signed( s );

    // bin reader
    for( PI32 u : uvals ) {
        int r = bw.read_unsigned();
        EXPECT_EQ( bw.empty(), 0 );
        EXPECT_EQ( bw.error(), 0 );
        EXPECT_EQ( r, u );
    }
    for( SI32 s : svals ) {
        int r = bw.read_signed();
        EXPECT_EQ( bw.empty(), s == - 5 );
        EXPECT_EQ( bw.error(), 0 );
        EXPECT_EQ( r, s );
    }
    (int)bw.read_signed();
    EXPECT_EQ( bw.error(), true );
    EXPECT_EQ( bw.empty(), true );
    EXPECT_EQ( bw.size(), 0 );
}

TEST( Buffers, compressed_of ) {
    // remove chuncks
    CbQueue cb;
    for( int i = 0; i < 16; ++i )
        cb.write_byte( i );
    EXPECT_EQ( to_string( cb ), "00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f" );

    cb.remove_chunks( std::vector<PT>({2,10}), std::vector<PT>({2,2}) );
    EXPECT_EQ( to_string( cb ), "00 01 04 05 06 07 08 09 0c 0d 0e 0f" );

    //
    CbQueue cn; BinStreamWithOffsets<CbQueue> bn( &cn );
    bn.write_byte( 0x40 );
    bn.beg_mark();
      bn.write_byte( 0x41 );
      bn.beg_mark();
        bn.write_byte( 0x42 );
      bn.end_mark();
      bn.write_byte( 0x43 );
    bn.end_mark();
    bn.write_byte( 0x44 );

    bn.crunch();
    EXPECT_EQ( to_string( cn ), "40 04 41 01 42 43 44" );
}

TEST( Buffers, utf8 ) {
    const char *data = "€123456";
    CbQueue c;
    c.write_some( data, strlen( data ) );
    CbString s = c;
    std::vector<unsigned> r;
    bool res = s.find_utf8( [ &r ]( unsigned v ) { r.push_back( v ); return false; } );
    EXPECT_EQ( res, false );
    EXPECT_EQ( to_string( r ), "14844588 49 50 51 52 53 54" );
}

TEST( Buffers, CbStringPtr ) {
    CbString cs;
    {
        CbQueue cq; BinStream<CbQueue> bq( &cq );
        bq << "123456";
        //PRINT( cq );

        cs = cq;
        //PRINT( cs );
    }

    CbStringPtr cp( cs );
    //PRINT( cp );
    BinStream<CbStringPtr> bp( &cp );

    CbString s = bp.read();
    EXPECT_EQ( std::string( s ), "123456" );
}

TEST( Buffers, write_some ) {
    CbQueue cq_1;

    {
        BBQ bq_1( &cq_1 );
        bq_1 << 1u;

        CbQueue cq_0;
        BBQ( &cq_0 ) << 3u << 4;

        bq_1 << 2u;
        bq_1.write_some( std::move( cq_0 ) ); //
    }
    EXPECT_EQ( to_string( cq_1 ), "01 02 03 04" );
}

TEST( Buffers, read_line ) {
    CbQueue cq; cq.write_some( "ab\ncd\n\nef" );
    CbString cs = std::move( cq );
    std::vector<std::string> expected({ "ab", "cd", "ef" });
    while ( CbString line = cs.read_line() ) {
        EXPECT_EQ( std::string( line ), expected[ 0 ] );
        expected.erase( expected.begin() );
    }
    EXPECT_EQ( expected.size(), 0 );
    EXPECT_EQ( cq.size(), 0 );
}

TEST( Buffers, starts_with ) {
    CbQueue cq; cq.write_some( "abcd" );
    CbString cs = std::move( cq );

    EXPECT_EQ( cs.starts_with( "ab" ), true );
    EXPECT_EQ( cs.starts_with( "abcd" ), true );
    EXPECT_EQ( cs.starts_with( "abcdef" ), false );

    EXPECT_EQ( cs.starts_with( "bc" ), false );
}

TEST( Buffers, PT_rw ) {
    CbQueue cq;
    BinStream<CbQueue> bq( &cq );
    for( PT var = 0; var < 258; ++var )
        bq << var;
    std::vector<PT> res, exp;
    for( PT var = 0; var < 258; ++var ) {
        res.push_back( bq.read() );
        exp.push_back( var );
    }
    EXPECT_EQ( res, exp );
}

TEST( Buffers, ST_rw ) {
    CbQueue cq;
    BinStream<CbQueue> bq( &cq );
    for( ST var = -256; var < 258; ++var )
        bq << var;
    std::vector<ST> res, exp;
    for( ST var = -256; var < 258; ++var ) {
        res.push_back( bq.read() );
        exp.push_back( var );
    }
    EXPECT_EQ( res, exp );
}


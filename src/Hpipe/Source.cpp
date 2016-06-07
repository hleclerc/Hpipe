#include "Source.h"
#include <stdlib.h>
#include <string.h>
#include <fstream>
using namespace std;
using namespace Hpipe;

Source::Source( const char *file ) : prev( 0 ) {
    ssize_t size_prov = strlen( file ) + 1;

    std::ifstream f( file );
    if ( f ) {
        f.seekg( 0, ios_base::end );
        ssize_t size_data = f.tellg() + ssize_t( 1 );
        f.seekg( 0, ios_base::beg );

        rese = (char *)malloc( size_prov + size_data );

        f.read( rese + size_prov, size_data - 1 );
        rese[ size_prov + size_data - 1 ] = 0;
        data = rese + size_prov;
    } else {
        rese = (char *)malloc( size_prov );
        data = 0;
    }

    // copy file -> provenance
    memcpy( rese, file, size_prov );
    provenance = rese;
}

Source::Source( const char *file, const char *text, bool need_cp ) : prev( 0 ) {
    if ( need_cp ) {
        ssize_t size_prov = strlen( file ) + 1;
        ssize_t size_data = strlen( text ) + 1;
        rese = (char *)malloc( size_prov + size_data );

        memcpy( rese, file, size_prov );
        provenance = rese;

        memcpy( rese + size_prov, text, size_data );
        data = rese + size_prov;
    } else {
        provenance = file;
        data = text;
        rese = 0;
    }
}

Source::~Source() {
    if ( rese )
        free( rese );
    delete prev;
}


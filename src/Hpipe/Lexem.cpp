#include "DotOut.h"
#include "Lexem.h"

#include <string.h>
#include <fstream>

using namespace std;

namespace Hpipe {

Lexem::Lexem( Type type, Source *source, const char *beg, const string str ) : beg( beg ), str( str ), type( type ), source( source ) {
    children[ 0 ] = 0;
    children[ 1 ] = 0;
    parent        = 0;
    next          = 0;
    prev          = 0;
    sibling       = 0;
}

void Lexem::write_to_stream( ostream &os ) const {
    os << str;
    if ( children[ 0 ] ) {
        os << "(" << *children[ 0 ];
        if ( children[ 1 ] )
            os << "," << *children[ 1 ];
        os << ")";
    }
    if ( next )
        os << *next;
}

int Lexem::display_dot( const char *f, const char *prg ) const {
    std::ofstream of( f );
    StreamSepMaker os( of );
    os << "digraph LexemMaker {";

    int m = write_dot( os, 0 );

    for( int i = 0; i <= m; ++i ) {
        os << "  " << i << " [ shape=plaintext ];";
        if ( i )
            os << "  " << i - 1 << " -> " << i << ";";
    }

    os << "}";
    of.close();

    return exec_dot( f, prg );
}

int Lexem::write_dot( StreamSepMaker &os, int p ) const {
    int res = p;

    *os.stream << "{ rank=same; " << p << " node_" << this << " [label=\"";
    dot_out( *os.stream, str );
    os << "\"] }";
    if ( next ) {
        os << "  node_" << this << " -> node_" << next << ";";
        res = max( res, next->write_dot( os, p ) );
    }
    for( int i = 0; i < 2; ++i ) {
        if ( const Lexem *c = children[ i ] ) {
            os << "  node_" << this << " -> node_" << c << " [color=\"green\"];";
            res = max( res, c->write_dot( os, p + 1 ) );
        }
    }

    if ( prev )
        os << "  node_" << prev << " -> node_" << this << " [color=\"yellow\"];";
    if ( parent )
        os << "  node_" << parent << " -> node_" << this << " [color=\"red\"];";
    return res;
}

int Lexem::ascii_val() const {
    if ( type == STRING and str.size() == 1 ) return str[ 0 ];
    if ( type == NUMBER ) return to_int();
    return -1;
}

bool Lexem::eq( Type _type, const std::string &val ) const {
    return type == _type and eq( val );
}

bool Lexem::eq( const std::string &val ) const {
    return str == val;
}

int Lexem::to_int() const {
    int res = 0;
    for( char c : str )
        res = 10 * res + ( c - '0' );
    return res;
}

std::ostream &operator<<( std::ostream &os, const Lexem &l ) {
    return os << l.str;
}

const Lexem *last( const Lexem *l ) {
    while ( l->next )
        l = l->next;
    return l;
}

} // namespace Hpipe

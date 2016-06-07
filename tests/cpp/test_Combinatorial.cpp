#include <Hpipe/Combinatorial.h>
#include <Hpipe/Check.h>
#include <Hpipe/Vec.h>
using namespace Hpipe;

void test_sweep() {
    Vec<int> a;
    a << 1 << 2 << 3 << 4;
    std::ostringstream ss[ 5 ];
    for( unsigned i = 0; i <= 4; ++i ) {
        combinatorial_find( a, i, [&]( const Vec<int> &p, const Vec<int> &q ) {
            ss[ i ] << "[" << p << "," << q << "]";
            return false;
        } );
    }

    CHECK( ss[ 0 ].str(), "[,1 2 3 4]" );
    CHECK( ss[ 1 ].str(), "[1,2 3 4][2,1 3 4][3,1 2 4][4,1 2 3]" );
    CHECK( ss[ 2 ].str(), "[1 2,3 4][1 3,2 4][1 4,2 3][2 3,1 4][2 4,1 3][3 4,1 2]" );
    CHECK( ss[ 3 ].str(), "[1 2 3,4][1 2 4,3][1 3 4,2][2 3 4,1]" );
    CHECK( ss[ 4 ].str(), "[1 2 3 4,]" );
}

void test_cond() {
    Vec<int> a;
    a << 1 << 2 << 3 << 4;
    std::ostringstream ss;

    CHECK( combinatorial_find( a, 2, [&]( const Vec<int> &p, const Vec<int> &q ) {
        ss << p;
        return true;
    } ), true );
    CHECK( ss.str(), "1 2" );
}
void test_large() {
    Vec<int> a, b;
    for( unsigned i = 0; i < 110; ++i )
        a << i;

    CHECK( combinatorial_find( a, 110, [&]( const Vec<int> &p, const Vec<int> &q ) {
        b = p;
        return true;
    } ), true );
    CHECK( b, a );
}

int main() {
    test_sweep();
    test_cond();
    test_large();
}



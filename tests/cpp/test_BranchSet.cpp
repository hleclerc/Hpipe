#include <Hpipe/BranchSet.h>
#include <Hpipe/Check.h>
using namespace Hpipe;

void test_interval() {
    Vec<BranchSet::Range> ranges;
    ranges.emplace_back( BranchSet::Range{  0,  30, nullptr, "A;",  1 } );
    ranges.emplace_back( BranchSet::Range{ 30,  60, nullptr, "B;",  2 } );
    ranges.emplace_back( BranchSet::Range{ 60,  90, nullptr, "C;", 99 } );
    ranges.emplace_back( BranchSet::Range{ 90, 120, nullptr, "D;",  3 } );

    BranchSet best_bs( ranges );
    CHECK( best_bs, "if ( *data >= 90 ) { D;/*3*/ } else { if ( *data < 60 ) { if ( *data < 30 ) { A;/*1*/ } else { B;/*2*/ } } else { C;/*99*/ } }" );
}

void test_value_1() {
    const char *a = "A;", *b = "B;";

    Vec<BranchSet::Range> ranges;
    ranges.emplace_back( BranchSet::Range{  0,  30, nullptr, a, 1 } );
    ranges.emplace_back( BranchSet::Range{ 30,  31, nullptr, b, 2 } );
    ranges.emplace_back( BranchSet::Range{ 31,  60, nullptr, a, 1 } );

    BranchSet best_bs( ranges );
    CHECK( best_bs, "if ( *data == 30 ) { B;/*2*/ } else { A;/*2*/ }" );
}

void test_value() {
    const char *a = "A;", *b = "B;", *c = "C;";

    Vec<BranchSet::Range> ranges;
    ranges.emplace_back( BranchSet::Range{  0,  30, nullptr, a,    1 } );
    ranges.emplace_back( BranchSet::Range{ 30,  31, nullptr, b,  100 } );
    ranges.emplace_back( BranchSet::Range{ 31,  32, nullptr, c, 1000 } );
    ranges.emplace_back( BranchSet::Range{ 32,  60, nullptr, a,    1 } );

    BranchSet best_bs( ranges );
    CHECK( best_bs, "if ( *data != 31 ) { if ( *data != 30 ) { A;/*2*/ } else { B;/*100*/ } } else { C;/*1000*/ }" );
}

void test_lor() {
    const char *a = "A;", *b = "B;", *c = "C;";

    Vec<BranchSet::Range> ranges;
    ranges.emplace_back( BranchSet::Range{  0,  30, nullptr, a,    1 } );
    ranges.emplace_back( BranchSet::Range{ 30,  31, nullptr, b,  100 } );
    ranges.emplace_back( BranchSet::Range{ 31,  32, nullptr, c, 1000 } );
    ranges.emplace_back( BranchSet::Range{ 32,  60, nullptr, a,    1 } );

    BranchSet best_bs( ranges );
    CHECK( best_bs, "if ( *data != 31 ) { if ( *data != 30 ) { A;/*2*/ } else { B;/*100*/ } } else { C;/*1000*/ }" );
}

int main() {
//    test_interval();
//    test_value_1();
//    test_value();
    test_lor();
}



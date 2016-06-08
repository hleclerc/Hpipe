#include <Hpipe/Check.h>
#include <Hpipe/Cond.h>
using namespace Hpipe;

int main() {
    Cond not_in_d( 'd' );

    CHECK( Cond( 'a' )                      , "'a'"      );
    CHECK( Cond( 'a' )    | Cond( 'c' )     , "'a'|'c'"  );
    CHECK( Cond( 'a' )    | Cond( 'b' )     , "'a'..'b'" );
    CHECK( Cond( 'a' )    | Cond( 'b', 255 ), "'a'..."   );
    CHECK( Cond( 0, 'a' ) | Cond( 'b' )     , "...'b'"   );

    CHECK( ( Cond( 'a' )      | Cond( 'c' )      ).ok_cpp( "v" ), "v == 'a' or v == 'c'" );
    CHECK( ( Cond( 'a', 'b' ) | Cond( 'd' )      ).ok_cpp( "v" ), "v >= 'a' and v <= 'd' and v != 'c'" );

    CHECK( ( Cond( 'b', 'c' ) | Cond( 'e' )      ).ok_cpp( "v", &not_in_d ), "v >= 'b' and v <= 'e'" );
    CHECK( ( Cond( 'b', 'c' ) | Cond( 'f', 'g' ) ).ok_cpp( "v", &not_in_d ), "v >= 'b' and v <= 'g' and v != 'e'" );

    CHECK( ( Cond( 'a', 'b' ) | Cond( 'e', 'f' ) ).ok_cpp( "v", &not_in_d ), "v >= 'a' and v <= 'f' and v != 'c'" );

    CHECK( ( ~ Cond( 'a' )                       ).ok_cpp( "v", &not_in_d ), "v != 'a'" );
    CHECK( ( ~( Cond( 'a' ) | Cond( 'z' ) )      ).ok_cpp( "v", &not_in_d ), "v != 'a' and v != 'z'" );
    CHECK( ( Cond( 0, 255 )                      ).ok_cpp( "v", &not_in_d ), "true" );
    CHECK( ( Cond()                              ).ok_cpp( "v", &not_in_d ), "false" );

    CHECK( ( Cond( 'c' ) | Cond( 'e', 255 )      ).ok_cpp( "v", &not_in_d ), "v >= 'c'" );
    CHECK( ( Cond( 'd' ) | Cond( 'z' )           ).ok_cpp( "v", &not_in_d ), "v == 'z'" );
    CHECK( ( ~ ( Cond( 'd' ) | Cond( 'z' ) )     ).ok_cpp( "v", &not_in_d ), "v != 'z'" );

    CHECK( Cond( 'c' ).ok_cpp( "v", &not_in_d ), "v == 'c'" );
    CHECK( Cond( 'c', 'e' ).ok_cpp( "v", &not_in_d ), "v >= 'c' and v <= 'e'" );
}


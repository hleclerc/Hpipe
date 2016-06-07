#include <Hpipe/FindVarInCode.h>
#include <Hpipe/Check.h>
using namespace Hpipe;

int main() {
    constexpr auto f = std::string::npos;

    CHECK( find_var_in_code( " data "          , "data", 0 ), 1 );
    CHECK( find_var_in_code( " datas "         , "data", 0 ), f );
    CHECK( find_var_in_code( " sdata "         , "data", 0 ), f );

    CHECK( find_var_in_code( " \"data\" "      , "data", 0 ), f );
    CHECK( find_var_in_code( " \"\"data "      , "data", 0 ), 3 );
    CHECK( find_var_in_code( " \"\\\"data\" "  , "data", 0 ), f );
    CHECK( find_var_in_code( " \"\\\\\"data\" ", "data", 0 ), 5 );

    CHECK( find_var_in_code( " \n data"        , "data", 0 ), 3 );
    CHECK( find_var_in_code( " // data"        , "data", 0 ), f );

    CHECK( find_var_in_code( " /* */ data"     , "data", 0 ), 7 );
    CHECK( find_var_in_code( " /* data */"     , "data", 0 ), f );

    CHECK( find_var_in_code( " foo . data"     , "data", 0 ), f );
    CHECK( find_var_in_code( " foo -> data"    , "data", 0 ), f );
    CHECK( find_var_in_code( " foo ::\n data"  , "data", 0 ), f );
    CHECK( find_var_in_code( " label: data"    , "data", 0 ), 8 );
    CHECK( find_var_in_code( " foo + data"     , "data", 0 ), 7 );
}



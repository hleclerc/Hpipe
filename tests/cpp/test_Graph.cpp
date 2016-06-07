#include <Hpipe/CppEmitter.h>
using namespace Hpipe;

void test( std::string machine, std::vector<std::pair<std::string,std::string>> test_strings ) {
    ErrorList error_list;
    Lexer lexer( error_list );

    lexer.read( new Source( "src/Hpipe/predef.sipe" ) );
    lexer.read( new Source( "internal", machine.c_str(), false ) );
    // lexer.root()->display_dot();

    CharGraph cg( lexer, lexer.find_machine( "main" ) );
    // cg.display_dot();

    InstructionGraph sg( &cg );
    // sg.display_dot();

    CppEmitter cp( &sg );
    cp.test( test_strings );
}

void test_cpp_preproc() {
    const char *machine =
            "main = '#include <' '>'"
            "";
}

int main() {
    // test( "main = 90 .. 98", {{ "abcdefgh", "OK" }} );

    // test( "main = 'ab'", {{ "abcdefgh", "OK" }} );
    // test( "main = 'a' 'bcd'** 'e'", {{ "abcdefgh", "OK" }} );
    // test( "main = 'a' 'bcd'** 'efgh'", {{ "abcdefgh", "OK" }} );
    // test( "main = 'a' ( 'bc' 'de' ) 'f'", {{ "abcdefgh", "OK" }} );
    // test( "main = 'a' 'bc' | 'de' 'f'", {{ "abcdefgh", "OK" }} );
    // test( "main = 'a' ( 'bc' { os << 'b'; } )** 'd'", {{ "abcbcd", "OK" }} );
    test( "main = 'a' ( 'bc' { os << 'b'; } )** 80", {{ "abcbcd", "bbKO" }} );

    // test( "main = 'A' ( 'bc' )* 'B' ( 'bc' )* 'B' ( 'bc' )** 'C'", {{ "abcbcd", "OK" }} ); // qu'est-ce qui dit de continuer ?
    // test( "main = 'A' ( 'bc' )* 'B'", {{ "abcbcd", "OK" }} ); // qu'est-ce qui dit de continuer ?
}



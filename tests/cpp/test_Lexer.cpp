#include <Hpipe/Lexer.h>
using namespace Hpipe;

int main() {
    ErrorList error_list;
    Lexer lexer( error_list );

    lexer.read( new Source( "internal", "main = 'a' .. 'z'", false ) );
    lexer.root()->display_dot( ".lex.dot" );
}


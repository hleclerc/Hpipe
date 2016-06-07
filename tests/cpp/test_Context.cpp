#include <Hpipe/CharGraph.h>
#include <Hpipe/Context.h>
#include <Hpipe/Check.h>
using namespace Hpipe;

int main() {
    ErrorList error_list;
    Lexer lexer( error_list );
    if ( not disp_lexem_graph )
        lexer.read( new Source( "src/Hpipe/predef.sipe" ) );
    lexer.read( new Source( "internal", "main = " ) );

    // char and transitions
    CharGraph cg( lexer, lexer.find_machine( "main" ) );
    if ( disp_char_graph )
        cg.display_dot();

    Context c;

}


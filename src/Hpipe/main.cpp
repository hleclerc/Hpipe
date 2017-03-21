//// nsmake alias predef.h (#txt_to_cpp.cpp predef.hpipe 'predef')
#include "CppEmitter.h"
#include "predef.h"
#include "Pool.h"
#include <string.h>
#include <iterator>
#include <fstream>
using namespace Hpipe;

#define PREPARG_FILE <Hpipe/args.h>
#include <PrepArg/usage.h>

int main( int argc, char **argv ) {
    // args
    #include <PrepArg/declarations.h>
    #include <PrepArg/parse.h>
    if ( beg_files < 0 )
        return usage( argv[ 0 ], "Please specify one or several input file", 2 );

    try {
        // lexems
        ErrorList error_list;
        Lexer lexer( error_list );
        Pool<Source> source_pool;
        if ( not disp_lexem_graph )
            lexer.read( source_pool.New( "src/Hpipe/predef.sipe", predef, false ) );
        for( int i = beg_files; i < argc; ++i )
            lexer.read( source_pool.New( argv[ i ] ) );
        if ( disp_lexem_graph ) {
            lexer.root()->display_dot();
            return 0;
        }
        if ( ! error_list )
            return 1;

        // char and transitions
        CharGraph cg( lexer );
        cg.read( lexer.find_machine( "main" ) );
        if ( disp_char_graph )
            cg.display_dot();
        if ( ! cg.ok )
            return 1;

        // instruction graph
        InstructionGraph ig;
        ig.stop_char   = strcmp( style, "C_STR" ) == 0 ? stop_char : -1;
        ig.boyer_moore = boyer_moore;
        ig.no_training = no_training;

        std::istringstream iss( disp_inst_graph );
        std::vector<std::string> disp( std::istream_iterator<std::string>{ iss }, std::istream_iterator<std::string>{} );
        ig.read( &cg, disp, disp_inst_pred, disp_trans_freq );

        // emitter
        CppEmitter ce;
        ce.trace_labels = trace_labels;

        if      ( strcmp( style, "HPIPE_BUFFER"  ) == 0 ) { ce.buffer_type = CppEmitter::BT_HPIPE_BUFFER; }
        else if ( strcmp( style, "BEG_END"       ) == 0 ) { ce.buffer_type = CppEmitter::BT_BEG_END     ; }
        else if ( strcmp( style, "C_STR"         ) == 0 ) { ce.buffer_type = CppEmitter::BT_C_STR       ; }
        else { std::cerr << "Unknown parse style ( " << style << " ). Possible values are HPIPE_BUFFER, BEG_END or C_STR" << std::endl; return 1; }

        ce.read( &ig );

        // output/test
        if ( benchmark )
            ce.bench( lexer.training_data() );

        if ( test )
            return ce.test( lexer.test_data() );

        if ( benchmark )
            return 0;

        StreamSepMaker ss( std::cout );
        std::ofstream fout;
        if ( output ) {
            fout.open( output );
            ss.stream = &fout;
        }

        ss << "// ----------------------------------------------------------------------------------------------------";
        ss << "#ifdef HPIPE_PRELIMINARIES";
        ce.write_preliminaries( ss );
        ss << "#endif // HPIPE_PRELIMINARIES";
        ss << "";

        ss << "// ----------------------------------------------------------------------------------------------------";
        ss << "#ifdef HPIPE_DECLARATIONS";
        ce.write_declarations( ss );
        ss << "#endif // HPIPE_DECLARATIONS";
        ss << "";

        ss << "// ----------------------------------------------------------------------------------------------------";
        ss << "#ifdef HPIPE_DEFINITIONS";
        ce.write_definitions( ss );
        ss << "#endif // HPIPE_DEFINITIONS";

        return 0;
    } catch ( const char *msg ) {
        std::cerr << msg << std::endl;
        return 1;
    }
}


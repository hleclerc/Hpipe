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

        // char and transitions
        const Lexem *margs;
        CharGraph cg( lexer, lexer.find_machine( margs, "main" ) );

        if ( disp_char_graph )
            cg.display_dot();

        // instruction (language independant)
        std::istringstream iss( disp_inst_graph );
        InstructionGraph sg( &cg, { std::istream_iterator<std::string>{ iss }, std::istream_iterator<std::string>{} }, disp_inst_pred, disp_trans_freq, boyer_moore );

        // output
        CppEmitter cp( &sg );

        if      ( strcmp( style, "BUFFER_IN_CLASS" ) == 0 ) { cp.buffer_type = CppEmitter::HPIPE_BUFFER; cp.in_class = true; }
        else if ( strcmp( style, "BUFFER"          ) == 0 ) { cp.buffer_type = CppEmitter::HPIPE_BUFFER; }
        else if ( strcmp( style, "BEG_END"         ) == 0 ) { cp.buffer_type = CppEmitter::BEGEND; }
        else if ( strcmp( style, "C_STR"           ) == 0 ) { cp.buffer_type = CppEmitter::C_STR; }
        else { std::cerr << "Unknown parse style ( " << style << " ). Possible values are BUFFER, BEG_END or C_STR" << std::endl; return 1; }

        if ( benchmark )
            cp.bench( lexer.training_data(), CppEmitter::BEGEND );

        if ( test )
            return cp.test( lexer.test_data() );

        if ( benchmark )
            return 0;

        StreamSepMaker ss( std::cout );
        std::ofstream fout;
        if ( output ) {
            fout.open( output );
            ss.stream = &fout;
        }

        ss << "#ifdef HPIPE_PRELIMINARIES";
        cp.write_preliminaries( ss );
        ss << "#endif // HPIPE_PRELIMINARIES";
        ss << "";

        ss << "#ifdef HPIPE_DECLARATIONS";
        cp.write_constants ( ss );
        cp.write_hpipe_data( ss, "HpipeData" );
        cp.write_parse_decl( ss, "HpipeData", "parse", args );
        ss << "#endif // HPIPE_DECLARATIONS";
        ss << "";

        ss << "#ifdef HPIPE_DEFINITIONS";
        cp.write_parse_def ( ss, "HpipeData", "parse", args );
        ss << "#endif // HPIPE_DEFINITIONS";
        return 0;

    } catch ( const char *msg ) {
        std::cerr << msg << std::endl;
        return 1;
    }
}

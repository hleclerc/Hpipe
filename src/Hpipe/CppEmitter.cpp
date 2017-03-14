#include "InstructionNextChar.h"
#include "FindVarInCode.h"
#include "CppEmitter.h"
#include "StreamSep.h"
#include "Assert.h"
#include <algorithm>
#include <string.h>
#include <fstream>

namespace Hpipe {

CppEmitter::CppEmitter( InstructionGraph *sg ) : root( sg->root() ), sg( sg ) {
    // default values
    end_char         = 0;
    buffer_type      = HPIPE_BUFFER;
    in_class         = false;
    test_mode        = false;
    trace_labels     = false;
    inst_to_go_if_ok = 0;

    // variables to be computed, max_mark_level, size_save_...
    need_mark     = false;
    size_save_glo = 0;
    size_save_loc = 0;
}

void CppEmitter::write_constants( StreamSepMaker &ss ) {
    ss << "// constants";
    ss << "static const unsigned RET_CONT      = 0;";
    ss << "static const unsigned RET_OK        = 1;";
    ss << "static const unsigned RET_KO        = 2;";
    ss << "static const unsigned RET_ENDED_OK  = 3;";
    ss << "static const unsigned RET_ENDED_KO  = 4;";
    ss << "static const unsigned RET_STOP_CONT = 5;";
}

void CppEmitter::write_preliminaries( StreamSepMaker &ss ) {
    ++Instruction::cur_op_id;
    root->apply_rec_rewind_l( [&]( Instruction *inst, unsigned rewind_level ) {
        if ( inst->is_a_mark() )
            need_mark = true;

        inst->reg_var( [&]( std::string type, std::string name ) {
            variables[ name ].type = type;
        }, this );

        if ( inst->save_in_loc_reg() >= 0 ) {
            unsigned &si = rewind_level ? size_save_loc : size_save_glo;
            si = std::max( si, unsigned( inst->save_in_loc_reg() + 1 ) );
        }
    } );

    //
    if ( not sg->cg->includes.empty() ) {
        ss << "";
        for( std::string inc : sg->cg->includes )
            ss << "#include <" << inc << ">";
    }
    for( const std::string &prel : sg->cg->preliminaries )
        ss << prel;

    switch ( buffer_type ) {
    case HPIPE_BUFFER:
        ss << "#ifndef HPIPE_BUFFER";
        ss << "#define HPIPE_BUFFER Hpipe::Buffer";
        ss << "#endif";
    }
}

void CppEmitter::write_hpipe_data( StreamSepMaker &ss, const std::string &name ) {
    ss << "struct " << name << " {";
    if ( interruptible() ) {
        ss << "    " << name << "() : inp_cont( 0 ) {}";
        if ( need_mark ) {
            ss << "    HPIPE_BUFFER        *pending_buf; ///< if we need to add the current buffer to a previous one";
            ss << "    HPIPE_BUFFER        *rw_buf;";
            ss << "    const unsigned char *rw_ptr;";
        }
        if ( size_save_glo )
            ss << "    unsigned char __save[ " << size_save_glo << " ];";
        for( const std::string &attr : sg->cg->attributes )
            ss << "    " << attr;
        ss << "    void *inp_cont;";
    }
    for( auto &p : variables )
        ss << "    " << p.second.type << " " << p.first << ";";
    ss << "};";
}

void CppEmitter::write_parse_decl( StreamSepMaker &ss, const std::string &hpipe_data_name, const std::string &func_name, const char *additional_args ) {
    std::string m = sg->methods();
    if ( m.size() )
        ss << m;

    switch ( buffer_type ) {
    case HPIPE_BUFFER:
        ss << "unsigned " << func_name << "( " << hpipe_data_name << " *sipe_data, HPIPE_BUFFER *buf, bool last_buf" << ( additional_args ? additional_args : "" ) << ", const unsigned char *data = 0, const unsigned char *end_m1 = 0 );";
        ss << "#ifdef HPIPE_DATA_IN_CLASS";
        ss << "unsigned " << func_name << "( HPIPE_BUFFER *buf, bool last_buf" << ( additional_args ? additional_args : "" ) << ", const unsigned char *data = 0, const unsigned char *end_m1 = 0 ) { return " << func_name << "( &hpipe_data, buf, last_buf" << ( additional_args ? additional_args : "" ) << ", data, end_m1 ); }";
        ss << "#endif // HPIPE_DATA_IN_CLASS";
        break;
    case BEGEND:
        ss << "unsigned " << func_name << "( " << hpipe_data_name << " *sipe_data, const unsigned char *data, const unsigned char *end_m1" << ( additional_args ? additional_args : "" ) << " );";
        ss << "#ifdef HPIPE_DATA_IN_CLASS";
        ss << "unsigned " << func_name << "( " << hpipe_data_name << " *sipe_data, const unsigned char *data, const unsigned char *end_m1" << ( additional_args ? additional_args : "" ) << " ) { return unsigned " << func_name << "( &hpipe_data, data, end_m1" << ( additional_args ? additional_args : "" ) << " ); }";
        ss << "#endif // HPIPE_DATA_IN_CLASS";
        break;
    case C_STR:
        ss << "unsigned " << func_name << "( " << hpipe_data_name << " *sipe_data, const unsigned char *data" << ( additional_args ? additional_args : "" ) << " );";
        ss << "#ifdef HPIPE_DATA_IN_CLASS";
        ss << "unsigned " << func_name << "( " << hpipe_data_name << " *sipe_data, const unsigned char *data" << ( additional_args ? additional_args : "" ) << " ) { return unsigned " << func_name << "( &hpipe_data, data" << ( additional_args ? additional_args : "" ) << " ); }";
        ss << "#endif // HPIPE_DATA_IN_CLASS";
        break;
    }
}

void CppEmitter::write_parse_def( StreamSepMaker &ss, const std::string &hpipe_data_name, const std::string &func_name, const char *additional_args ) {
    StreamSepMaker nss( *ss.stream, ss.beg + "    " );

    switch ( buffer_type ) {
    case HPIPE_BUFFER:
        ss << "#ifndef HPIPE_METHOD_PREFIX";
        ss << "#define HPIPE_METHOD_PREFIX";
        ss << "#endif";
        if ( in_class ) {
            ss << "unsigned HPIPE_METHOD_PREFIX " << func_name << "( HPIPE_BUFFER *buf, bool last_buf" << ( additional_args ? additional_args : "" ) << ", const unsigned char *data, const unsigned char *end_m1 ) {";
            ss << "    " << hpipe_data_name << " *sipe_data = &hpipe_data;";
        } else
            ss << "unsigned HPIPE_METHOD_PREFIX " << func_name << "( " << hpipe_data_name << " *sipe_data, HPIPE_BUFFER *buf, bool last_buf" << ( additional_args ? additional_args : "" ) << ", const unsigned char *data, const unsigned char *end_m1 ) {";
        nss << "if ( ! data ) data = buf->data;";
        nss << "if ( ! end_m1 ) end_m1 = buf->data - 1 + buf->used;";
        break;
    case BEGEND:
        ss << "unsigned HPIPE_METHOD_PREFIX " << func_name << "( " << hpipe_data_name << " *sipe_data, const unsigned char *data, const unsigned char *end_m1" << ( additional_args ? additional_args : "" ) << " ) {";
        if ( need_mark )
            nss << "const unsigned char *rw_ptr;";
        break;
    case C_STR:
        ss << "unsigned HPIPE_METHOD_PREFIX " << func_name << "( " << hpipe_data_name << " *sipe_data, const unsigned char *data" << ( additional_args ? additional_args : "" ) << " ) {";
        if ( need_mark )
            nss << "const unsigned char *rw_ptr;";
        break;
    }

    if ( size_save_loc or ( size_save_glo and not interruptible() ) )
        ss << "    unsigned char save[ " << std::max( size_save_loc, size_save_glo ) << " ];";
    if ( test_mode )
        ss << "    unsigned cpt = 0;";
    if ( interruptible() ) {
        ss << "    if ( sipe_data->inp_cont ) goto *sipe_data->inp_cont;";
    }

    nb_cont_label = 0;
    nb_id_gen     = 0;
    write_parse_body( nss, root );

    ss << "}";
}


void CppEmitter::write_parse_body( StreamSepMaker &ss, Instruction *root ) {
    std::ostringstream end_stream;
    StreamSepMaker es( end_stream, ss.beg );

    // ordering
    ++Instruction::cur_op_id;
    Vec<Instruction *> ordering;
    get_ordering( ordering, root );

    // update id_gen (for now, we set id_gen only for transition to the "past" -- in ordering)
    for( Instruction *inst : ordering )
        inst->id_gen = inst->always_need_id_gen( this ) ? ++nb_id_gen : 0;
    for( Instruction *inst : ordering )
        for( Transition &t : inst->next )
            if ( t.inst->num_ordering <= inst->num_ordering and not t.inst->id_gen )
                t.inst->id_gen = ++nb_id_gen;

    // go
    for( Instruction *inst : ordering ) {
        // write a label if neceassary
        if ( inst->id_gen )
            ss.rm_beg( 2 ) << "l_" << inst->id_gen << ":" << ( trace_labels ? " std::cout << \"l_" + to_string( inst->id_gen ) + " \" << __LINE__ << std::endl;" : "" );

        // write instruction content
        inst->write_cpp( ss, es, this );
    }

    *ss.stream << end_stream.str();
}

void CppEmitter::get_ordering( Vec<Instruction *> &ordering, Instruction *inst ) {
    if ( inst->op_id == Instruction::cur_op_id )
        return;
    inst->op_id = Instruction::cur_op_id;

    inst->num_ordering = ordering.size();
    inst->id_gen = 0;

    ordering << inst;

    for( Transition &t : inst->next )
        get_ordering( ordering, t.inst );
}

int CppEmitter::test( const std::vector<Lexer::TestData> &tds ) {
    test_mode = true;

    std::ofstream fout( "out.cpp" );
    StreamSepMaker ss( fout );
    StreamSepMaker ns( fout, "    " );
    ss << "#define HPIPE_CHECK_ALIVE_BUF";
    ss << "#define HPIPE_TEST";
    ss << "";
    ss << "#include <Hpipe/CbStringPtr.h>";
    ss << "#include <Hpipe/Print.h>";
    ss << "#include <iostream>";
    ss << "#include <sstream>";

    ss << "#include <Hpipe/CbString.cpp>";
    ss << "#include <Hpipe/Assert.cpp>";

    write_preliminaries( ss );

    ss << "";
    ss << "int Hpipe::Buffer::nb_alive_bufs = 0;";
    { // for( buffer_type = buffer_type; buffer_type < 3; ++buffer_type )
        ss << "";
        ss << "struct Test_" << buffer_type << " {";

        write_constants ( ns );
        write_hpipe_data( ns );
        write_parse_decl( ns );

        ss << "    int exec( const unsigned char *name, const unsigned char *data, unsigned size, std::string expected ) {";
        switch ( buffer_type ) {
        case HPIPE_BUFFER:
            ss << "        int res = RET_CONT;";
            ss << "        { HpipeData hd;";
            ss << "        for( unsigned i = 0; i < size; ++i ) {";
            ss << "            Hpipe::Buffer *buf = HPIPE_BUFFER::New( 17 );";
            ss << "            buf->data[ 0 ] = data[ i ];";
            ss << "            buf->used = 1;";
            ss << "            ";
            ss << "            res = parse( &hd, buf, i + 1 == size );";
            ss << "            HPIPE_BUFFER::dec_ref( buf );";
            ss << "            ";
            ss << "            if ( res != RET_CONT )";
            ss << "                break;";
            ss << "        }";
            ss << "        if ( not size ) {";
            ss << "            Hpipe::Buffer *buf = HPIPE_BUFFER::New( 0 );";
            ss << "            res = parse( &hd, buf, true );";
            ss << "            HPIPE_BUFFER::dec_ref( buf );";
            ss << "        }";
            ss << "        switch ( res ) {";
            ss << "        case RET_CONT: if ( os.str().size() ) os << ' '; os << \"status=CNT\"; break;"; //
            ss << "        case RET_OK  : if ( os.str().size() ) os << ' '; os << \"status=OK\" ; break;";
            ss << "        case RET_KO  : if ( os.str().size() ) os << ' '; os << \"status=KO\" ; break;";
            ss << "        }";
            ss << "        } if ( Hpipe::Buffer::nb_alive_bufs ) os << \" nb_remaining_bufs=\" << Hpipe::Buffer::nb_alive_bufs;";
            break;
        case BEGEND:
            ss << "        HpipeData hd;";
            ss << "        switch ( parse( &hd, data, data + size - 1 ) ) {";
            ss << "        case RET_CONT: if ( os.str().size() ) os << ' '; os << \"status=CNT\"; break;"; //
            ss << "        case RET_OK  : if ( os.str().size() ) os << ' '; os << \"status=OK\" ; break;";
            ss << "        case RET_KO  : if ( os.str().size() ) os << ' '; os << \"status=KO\" ; break;";
            ss << "        }";
            break;
        case C_STR:
            ss << "        HpipeData hd;";
            ss << "        switch ( parse( &hd, data ) ) {";
            ss << "        case RET_CONT: if ( os.str().size() ) os << ' '; os << \"status=CNT\"; break;"; //
            ss << "        case RET_OK  : if ( os.str().size() ) os << ' '; os << \"status=OK\" ; break;";
            ss << "        case RET_KO  : if ( os.str().size() ) os << ' '; os << \"status=KO\" ; break;";
            ss << "        }";
            break;
        }
        ss << "        std::cout << \"  \" << name << \" -> \" << ( os.str() == expected ? \"(OK) \" : \"(BAD) \" ) << os.str() << std::endl;";
        ss << "        return os.str() != expected;";
        ss << "    }";
        ss << "    std::ostringstream os;";
        ss << "    unsigned cpt = 0;";
        ss << "};";
        ss << "#define HPIPE_METHOD_PREFIX Test_" << buffer_type << "::";
        write_parse_def( ss );
        ss << "#undef HPIPE_METHOD_PREFIX";
    }


    ss << "int main() {";
    for( const Lexer::TestData &td : tds ) {
        *ss.stream << "    static unsigned char name_" << &td << "[] = { "; for( char c : td.name ) *ss.stream << (int)c << ","; *ss.stream << " 0 };\n";
        *ss.stream << "    static unsigned char inp_"  << &td << "[] = { "; for( char c : td.inp  ) *ss.stream << (int)c << ","; *ss.stream << " 0 };\n";
        *ss.stream << "    static unsigned char out_"  << &td << "[] = { "; for( char c : td.out  ) *ss.stream << (int)c << ","; *ss.stream << " 0 };\n";
    }
    ss << "    int res = 0;";
    for( const Lexer::TestData &td : tds )
        for( unsigned i = buffer_type; i <= buffer_type; ++i)
            ss << "    res |= Test_" << i << "().exec( name_" << &td << ", inp_" << &td << ", " << td.inp.size() << ", std::string( (const char *)out_" << &td << ", " << td.out.size() << " ) );";
    ss << "    return res;";
    ss << "}";
    fout.close();

    return system( "g++ -g3 -std=c++14 -Isrc/ -o out out.cpp && ./out" );
}


bool _exec( std::string cmd, bool disp = true ) {
    if ( disp )
        PRINTE( cmd );
    return system( cmd.c_str() ) == 0;
}

bool CppEmitter::bench( const std::vector<Lexer::TrainingData> &tds, int type ) {
    test_mode = true;

    std::ofstream fout( "bench.cpp" );
    StreamSepMaker ss( fout );
    StreamSepMaker ns( fout, "    " );
    ss << "#include <Hpipe/CbStringPtr.h>";
    ss << "#include <Hpipe/Print.h>";
    ss << "#include <iostream>";
    ss << "#include <sstream>";
    ss << "#include <ctime>";
    ss << "";
    write_preliminaries( ss );
    // ss << "#define HPIPE_TEST";
    { // for( buffer_type = 0; buffer_type < 3; ++buffer_type )
        ss << "";
        ss << "struct Bench_" << buffer_type << " {";

        write_constants    ( ns );
        write_hpipe_data   ( ns );
        write_parse_decl   ( ns );

        ss << "    void exec( const unsigned char *name, const unsigned char *data, unsigned size, const char *display ) {";
        switch ( buffer_type ) {
        case HPIPE_BUFFER:
            ss << "        Hpipe::Buffer *buf = Hpipe::Buffer::New( size );";
            ss << "        memcpy( buf->data, data, size );";
            ss << "        buf->used = size;";
            ss << "        auto t0 = std::clock();";
            ss << "        for( unsigned var = 0; var < 1000000; ++var ) {";
            ss << "            HpipeData hd;";
            ss << "            parse( &hd, buf, true );";
            ss << "        }";
            ss << "        auto t1 = std::clock();";
            ss << "        HPIPE_BUFFER::dec_ref( buf );";
            break;
        case BEGEND:
            ss << "        auto t0 = std::clock();";
            ss << "        for( unsigned var = 0; var < 1000000; ++var ) {";
            ss << "            HpipeData hd;";
            ss << "            parse( &hd, data, data - 1 + size );";
            ss << "        }";
            ss << "        auto t1 = std::clock();";
            break;
        case C_STR:
            ss << "        auto t0 = std::clock();";
            ss << "        for( unsigned var = 0; var < 1000000; ++var ) {";
            ss << "            HpipeData hd;";
            ss << "            parse( &hd, data );";
            ss << "        }";
            ss << "        auto t1 = std::clock();";
            break;
        }
        ss << "        if ( display )";
        ss << "            std::cout << name << \", \" << display << \" -> \" << double( t1 - t0 ) / CLOCKS_PER_SEC << std::endl;";
        ss << "    }";
        ss << "    std::ostringstream os;";
        ss << "    unsigned cpt = 0;";
        ss << "};";

        ss << "#define HPIPE_METHOD_PREFIX Bench_" << buffer_type << "::";
        write_parse_def( ss );
        ss << "#undef HPIPE_METHOD_PREFIX";
    }


    ss << "int main( int argc, char **argv ) {";
    for( const Lexer::TrainingData &td : tds ) {
        *ss.stream << "    static unsigned char name_" << &td << "[] = { "; for( char c : td.name ) *ss.stream << (int)c << ","; *ss.stream << " 0 };\n";
        *ss.stream << "    static unsigned char inp_"  << &td << "[] = { "; for( char c : td.inp  ) *ss.stream << (int)c << ","; *ss.stream << " 0 };\n";
        *ss.stream << "    static double freq_"        << &td << " = " << td.freq << ";\n";
    }
    for( const Lexer::TrainingData &td : tds )
        ss << "    Bench_" << buffer_type << "().exec( name_" << &td << ", inp_" << &td << ", " << td.inp.size() << ", argc > 1 ? argv[ 1 ] : 0 );";
    ss << "}";
    fout.close();

    bool clang = false;
    if ( clang ) {
        std::string gpp = "clang++ -O3 -march=native -std=c++14 -Isrc/ -o bench bench.cpp";
        return _exec( gpp ) && _exec( "echo without profile", false ) && _exec( "./bench 'without profile'" ) &&
               _exec( gpp + " -fprofile-instr-generate" ) && _exec( "./bench -nodisp" ) &&
               _exec( "llvm-profdata-3.8 merge -output=default.profdata default.profraw" ) &&
               _exec( gpp + " -fprofile-instr-use"      ) && _exec( "./bench 'with profile'" );
    }

    //std::string gpp = "g++ -O6 -g3 -fno-reorder-blocks -march=native -std=c++14 -Isrc/ -o bench bench.cpp";
    std::string gpp = "g++ -O6 -g3 -march=native -std=c++14 -Isrc/ -o bench bench.cpp";
    return _exec( gpp ) && _exec( "./bench 'without profile'" )
        && _exec( gpp + " -fprofile-generate" ) && _exec( "./bench" )
        && _exec( gpp + " -fprofile-use"      ) && _exec( "./bench 'with profile'" )
            ; // -fno-reorder-blocks-and-partition
}

//void CppEmitter::write_code( StreamSepMaker &ss, std::string str, const std::string &repl ) {
//    for( auto &p : variables ) {
//        for( std::string::size_type pos = 0; ; ) {
//            pos = find_var_in_code( str, p.first, pos );
//            if ( pos == std::string::npos )
//                break;
//            str = str.replace( pos, 0, "sipe_data->" );
//            pos += 11 + p.first.size();
//        }
//    }
//    if ( repl.size() )
//        str = repl_data( str, repl );

//    ss << str;
//}

std::string CppEmitter::repl_data( std::string code, const std::string &repl ) {
    using T = std::string::size_type;

    for( T pos = 0; ; ) {
        pos = find_var_in_code( code, "data", pos );
        if ( pos == std::string::npos )
            break;
        code.replace( pos, 4, repl );
        pos += repl.size();
    }

    return code;
}

} // namespace Hpipe

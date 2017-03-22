#include "InstructionNextChar.h"
#include "FindVarInCode.h"
#include "CppEmitter.h"
#include "StreamSep.h"
#include "Assert.h"
#include <algorithm>
#include <string.h>
#include <fstream>

namespace Hpipe {

namespace {

bool _exec( std::string cmd, bool disp = true ) {
    if ( disp )
        PRINTE( cmd );
    return system( cmd.c_str() ) == 0;
}

}

CppEmitter::CppEmitter() {
    // default values
    stop_char          = 0;
    buffer_type        = BT_HPIPE_BUFFER;
    trace_labels       = false;

    // variables to be computed
    size_save_glo      = 0;
    size_save_loc      = 0;
}

void CppEmitter::read( InstructionGraph *sg ) {
    for( const std::string &str : sg->preliminaries ) preliminaries.push_back_unique( str );
    for( const std::string &str : sg->includes      ) includes     .push_back_unique( str );
    for( const std::string &str : sg->methods       ) methods      .push_back_unique( str );
    for( const auto &p          : sg->variables     ) variables[ p.first ] = p.second;

    // ordering
    ++Instruction::cur_op_id;
    Vec<Instruction *> ordering;
    get_ordering( ordering, sg->root() );

    // update id_gen (for now, we set id_gen only for transition to the "past" -- in ordering)
    nb_id_gen = 0;
    for( Instruction *inst : ordering )
        inst->id_gen = inst->always_need_id_gen( this ) ? ++nb_id_gen : 0;
    for( Instruction *inst : ordering )
        for( Transition &t : inst->next )
            if ( t.inst->num_ordering <= inst->num_ordering and not t.inst->id_gen )
                t.inst->id_gen = ++nb_id_gen;

    // go
    nb_cont_label = 0;
    std::ostringstream beg_stream;
    std::ostringstream end_stream;
    StreamSepMaker ss( beg_stream, "    " );
    StreamSepMaker es( end_stream, "    " );
    for( Instruction *inst : ordering ) {
        // write a label if neceassary
        if ( inst->id_gen )
            write_label( ss, inst->id_gen );

        // write instruction content
        inst->write_cpp( ss, es, this );
    }

    parse_content = beg_stream.str() + end_stream.str();
}

void CppEmitter::write_preliminaries( StreamSepMaker &ss ) {
    for( const std::string &str : includes ) {
        if ( str.size() && ( str[ 0 ] == '<' || str[ 0 ] == '"' ) )
            ss << "#include " << str << "";
        else
            ss << "#include <" << str << ">";
    }

    for( const std::string &str : preliminaries )
        ss << str;
}

void CppEmitter::write_declarations( StreamSepMaker &ss ) {
    // constants
    ss << "#ifndef HPIPE_RET_CONSTANTS";
    ss << "#define HPIPE_RET_CONSTANTS";
    ss << "enum {";
    ss << "    RET_CONT      = 0,";
    ss << "    RET_OK        = 1,";
    ss << "    RET_KO        = 2,";
    ss << "    RET_ENDED_OK  = 3,";
    ss << "    RET_ENDED_KO  = 4,";
    ss << "    RET_STOP_CONT = 5";
    ss << "};";
    ss << "#endif // HPIPE_RET_CONSTANTS";

    ss << "";
    ss << "#ifndef HPIPE_ADDITIONAL_ARGS";
    ss << "#define HPIPE_ADDITIONAL_ARGS";
    ss << "#endif // HPIPE_ADDITIONAL_ARGS";

    ss << "";
    ss << "#ifndef HPIPE_PARSE_FUNC_NAME";
    ss << "#define HPIPE_PARSE_FUNC_NAME parse";
    ss << "#endif // HPIPE_PARSE_FUNC_NAME";

    ss << "";
    ss << "#ifndef HPIPE_DATA_STRUCT_NAME";
    ss << "#define HPIPE_DATA_STRUCT_NAME HpipeData";
    ss << "#endif // HPIPE_DATA_STRUCT_NAME";

    ss << "";
    ss << "#ifndef HPIPE_DATA_CTOR_NAME";
    ss << "#define HPIPE_DATA_CTOR_NAME init_##HPIPE_DATA_STRUCT_NAME";
    ss << "#endif // HPIPE_DATA_CTOR_NAME";

    ss << "";
    ss << "#ifndef HPIPE_CHAR_T";
    ss << "#define HPIPE_CHAR_T unsigned char";
    ss << "#endif // HPIPE_CHAR_T";

    if ( buffer_type == BT_HPIPE_BUFFER || buffer_type == BT_HPIPE_CB_STRING_PTR ) {
        ss << "";
        ss << "#ifndef HPIPE_BUFF_T";
        ss << "#define HPIPE_BUFF_T Hpipe::Buffer";
        ss << "#endif";
    }

    // constructor preparation
    std::ostringstream cs;
    StreamSepMaker cm( cs, "    " );
    if ( interruptible() )
        cm << "hpipe_data->inp_cont = 0;";
    for( auto &p : variables )
        if ( p.second.default_value.size() )
            cm << "hpipe_data->" << p.first << " = " << p.second.default_value << ";";
    ctor = cs.str();

    // struct HpipeData
    ss << "";
    ss << "struct HPIPE_DATA_STRUCT_NAME {";
    //
    if ( ctor.size() ) {
        ss << "  #ifdef __cplusplus";
        ss << "  HPIPE_DATA_STRUCT_NAME() { HPIPE_DATA_STRUCT_NAME *hpipe_data = this;\n" << ctor << "  }";
        ss << "  #endif // __cplusplus";
    }
    if ( size_save_glo )
        ss << "  HPIPE_CHAR_T __save[ " << size_save_glo << " ];";
    for( auto &p : variables )
        ss << "  " << p.second.type << " " << p.first << ";";
    if ( interruptible() )
        ss << "  void *inp_cont;";
    ss << "};";

    // constructor
    ss << "static void HPIPE_DATA_CTOR_NAME( HPIPE_DATA_STRUCT_NAME *hpipe_data ) {";
    if ( ctor.size() )
        *ss.stream << ctor;
    ss << "}";

    // user methods
    for( const std::string &str : methods )
        ss << str;

    // parse decl
    std::ostringstream args;
    args << "HPIPE_ADDITIONAL_ARGS ";
    switch ( buffer_type ) {
    case BT_HPIPE_CB_STRING_PTR: args << "const HPIPE_BUFF_T *buf, size_t off, size_t end"; break;
    case BT_HPIPE_BUFFER       : args << "HPIPE_BUFF_T *buf, bool last_buf, const HPIPE_CHAR_T *data = 0, const HPIPE_CHAR_T *end_m1 = 0"; break;
    case BT_BEG_END            : args << "const HPIPE_CHAR_T *data, const HPIPE_CHAR_T *end_m1"; break;
    case BT_C_STR              : args << "const HPIPE_CHAR_T *data"; break;
    }
    ss << "unsigned HPIPE_PARSE_FUNC_NAME( " << args.str() << " );";
}

void CppEmitter::write_definitions( StreamSepMaker &ss ) {
    // defines
    ss << "#ifndef HPIPE_DATA";
    ss << "#define HPIPE_DATA hpipe_data";
    ss << "#endif // HPIPE_DATA";

    ss << "";
    ss << "#ifndef HPIPE_DEFINITION_PREFIX";
    ss << "#define HPIPE_DEFINITION_PREFIX";
    ss << "#endif // HPIPE_DEFINITION_PREFIX";

    ss << "";
    ss << "#ifndef HPIPE_PARSE_FUNC_NAME";
    ss << "#define HPIPE_PARSE_FUNC_NAME parse";
    ss << "#endif // HPIPE_PARSE_FUNC_NAME";

    ss << "";
    ss << "#ifndef HPIPE_ADDITIONAL_ARGS";
    ss << "#define HPIPE_ADDITIONAL_ARGS";
    ss << "#endif // HPIPE_ADDITIONAL_ARGS";

    if ( need_buf() ) {
        ss << "";
        ss << "#ifndef HPIPE_BUFF_T";
        ss << "#define HPIPE_BUFF_T Hpipe::Buffer";
        ss << "#endif";
        ss << "";
        ss << "#ifndef HPIPE_BUFF_T__DEC_REF";
        ss << "#define HPIPE_BUFF_T__DEC_REF( buf ) HPIPE_BUFF_T::dec_ref( buf )";
        ss << "#endif";
        ss << "";
        ss << "#ifndef HPIPE_BUFF_T__INC_REF";
        ss << "#define HPIPE_BUFF_T__INC_REF( buf ) HPIPE_BUFF_T::inc_ref( buf )";
        ss << "#endif";
        ss << "";
        ss << "#ifndef HPIPE_BUFF_T__INC_REF_N";
        ss << "#define HPIPE_BUFF_T__INC_REF_N( buf, N ) HPIPE_BUFF_T::inc_ref( buf, N )";
        ss << "#endif";
        ss << "";
        ss << "#ifndef HPIPE_BUFF_T__SKIP";
        ss << "#define HPIPE_BUFF_T__SKIP( buf, ptr, N ) HPIPE_BUFF_T::skip( buf, ptr, N )";
        ss << "#endif";
        ss << "";
        ss << "#ifndef HPIPE_BUFF_T__SKIP_N";
        ss << "#define HPIPE_BUFF_T__SKIP_N( buf, ptr, N, K ) HPIPE_BUFF_T::skip( buf, ptr, N, K )";
        ss << "#endif";
        ss << "";
        ss << "#ifndef HPIPE_BUFF_T__DEC_REF_UPTO";
        ss << "#define HPIPE_BUFF_T__DEC_REF_UPTO( beg, end ) beg->dec_ref_upto( end )";
        ss << "#endif";
        ss << "";
        ss << "#ifndef HPIPE_BUFF_T__DEC_REF_UPTO_N";
        ss << "#define HPIPE_BUFF_T__DEC_REF_UPTO_N( beg, end, K ) beg->dec_ref_upto( end, K )";
        ss << "#endif";
    }

    // preparation for function signature
    std::ostringstream args;
    args << "HPIPE_ADDITIONAL_ARGS ";
    switch ( buffer_type ) {
    case BT_HPIPE_CB_STRING_PTR: args << "const HPIPE_BUFF_T *buf, size_t off, size_t end"; break;
    case BT_HPIPE_BUFFER       : args << "HPIPE_BUFF_T *buf, bool last_buf, const HPIPE_CHAR_T *data, const HPIPE_CHAR_T *end_m1"; break;
    case BT_BEG_END            : args << "const HPIPE_CHAR_T *data, const HPIPE_CHAR_T *end_m1"; break;
    case BT_C_STR              : args << "const HPIPE_CHAR_T *data"; break;
    }

    // function signature
    ss << "";
    ss << "unsigned HPIPE_DEFINITION_PREFIX HPIPE_PARSE_FUNC_NAME( " << args.str() << " ) {";

    // warm up
    StreamSepMaker nss( *ss.stream, ss.beg + "    " );
    switch ( buffer_type ) {
    case BT_HPIPE_CB_STRING_PTR:
        nss << "while ( buf && off >= buf->used ) { off -= buf->used; end -= buf->used; buf = buf->next; }";
        nss << "const HPIPE_CHAR_T *data = buf ? buf->data + off : (const HPIPE_CHAR_T *)1;";
        nss << "const HPIPE_CHAR_T *end_m1 = buf ? buf->data - 1 + ( end > buf->used ? buf->used : end ) : 0;";
        break;
    case BT_HPIPE_BUFFER:
        nss << "if ( ! data ) data = buf->data;";
        nss << "if ( ! end_m1 ) end_m1 = buf->data - 1 + buf->used;";
        break;
    }
    if ( size_save_loc )
        nss << "HPIPE_CHAR_T save[ " << size_save_loc << " ];";
    for( const std::string &str : loc_vars )
        nss << str;
    if ( interruptible() )
        nss << "if ( HPIPE_DATA.inp_cont ) goto *HPIPE_DATA.inp_cont;";

    // content
    *ss.stream << parse_content;

    // end
    ss << "}";
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
    std::ofstream fout( "out.cpp" );
    StreamSepMaker ss( fout );
    ss << "#define HPIPE_CHECK_ALIVE_BUF";
    ss << "#define HPIPE_TEST";
    ss << "";
    ss << "#include <Hpipe/CbStringPtr.h>";
    ss << "#include <Hpipe/Print.h>";
    ss << "#include <iostream>";
    ss << "#include <sstream>";

    ss << "";
    ss << "int Hpipe::Buffer::nb_alive_bufs = 0;";

    ss << "";
    ss << "#ifdef INCLUDE_CPP";
    ss << "#include <Hpipe/CbString.cpp>";
    ss << "#include <Hpipe/CmString.cpp>";
    ss << "#include <Hpipe/Assert.cpp>";
    ss << "#endif // INCLUDE_CPP";

    ss << "";
    ss << "#define HPIPE_DEFINITION_PREFIX Test::";

    ss << "";
    write_preliminaries( ss );

    ss << "";
    ss << "struct Test {";
    StreamSepMaker ns( fout, "    " );
    write_declarations( ns );
    ns << "int exec( const unsigned char *name, const HPIPE_CHAR_T *data, unsigned size, std::string expected );";
    ns << "std::ostringstream os;";
    ns << "HpipeData hpipe_data;";
    ns << "unsigned cpt = 0;";
    ss << "};";

    ss << "";
    ss << "int Test::exec( const unsigned char *name, const HPIPE_CHAR_T *data, unsigned size, std::string expected ) {";
    switch ( buffer_type ) {
    case BT_HPIPE_CB_STRING_PTR:
        ns << "int res = RET_CONT;";
        ns << "Hpipe::Buffer *beg = 0, *old = 0;";
        ns << "for( unsigned i = 0; i < size; ++i ) {";
        ns << "    Hpipe::Buffer *buf = HPIPE_BUFF_T::New( 1, old );";
        ns << "    old = buf; if ( ! beg ) beg = buf;";
        ns << "    buf->data[ 0 ] = data[ i ];";
        ns << "    buf->used = 1;";
        ns << "}";
        ns << "res = HPIPE_PARSE_FUNC_NAME( beg, 0, size );";
        ns << "while( beg ) {";
        ns << "    Hpipe::Buffer *old = beg;";
        ns << "    beg = beg->next;";
        ns << "    HPIPE_BUFF_T::dec_ref( old );";
        ns << "}";
        ns << "switch ( res ) {";
        ns << "case RET_CONT: if ( os.str().size() ) os << ' '; os << \"status=CNT\"; break;"; //
        ns << "case RET_OK  : if ( os.str().size() ) os << ' '; os << \"status=OK\" ; break;";
        ns << "case RET_KO  : if ( os.str().size() ) os << ' '; os << \"status=KO\" ; break;";
        ns << "}";
        break;
    case BT_HPIPE_BUFFER:
        ns << "int res = RET_CONT;";
        ns << "for( unsigned i = 0; i < size; ++i ) {";
        ns << "    Hpipe::Buffer *buf = HPIPE_BUFF_T::New( 1 );";
        ns << "    buf->data[ 0 ] = data[ i ];";
        ns << "    buf->used = 1;";
        ns << "    ";
        ns << "    res = HPIPE_PARSE_FUNC_NAME( buf, i + 1 == size );";
        ns << "    HPIPE_BUFF_T::dec_ref( buf );";
        ns << "    ";
        ns << "    if ( res != RET_CONT )";
        ns << "        break;";
        ns << "}";
        ns << "if ( not size ) {";
        ns << "    Hpipe::Buffer *buf = HPIPE_BUFF_T::New( 0 );";
        ns << "    res = HPIPE_PARSE_FUNC_NAME( buf, true );";
        ns << "    HPIPE_BUFF_T::dec_ref( buf );";
        ns << "}";
        ns << "switch ( res ) {";
        ns << "case RET_CONT: if ( os.str().size() ) os << ' '; os << \"status=CNT\"; break;"; //
        ns << "case RET_OK  : if ( os.str().size() ) os << ' '; os << \"status=OK\" ; break;";
        ns << "case RET_KO  : if ( os.str().size() ) os << ' '; os << \"status=KO\" ; break;";
        ns << "}";
        break;
    case BT_BEG_END:
        ns << "switch ( HPIPE_PARSE_FUNC_NAME( data, data + size - 1 ) ) {";
        ns << "case RET_CONT: if ( os.str().size() ) os << ' '; os << \"status=CNT\"; break;"; //
        ns << "case RET_OK  : if ( os.str().size() ) os << ' '; os << \"status=OK\" ; break;";
        ns << "case RET_KO  : if ( os.str().size() ) os << ' '; os << \"status=KO\" ; break;";
        ns << "}";
        break;
    case BT_C_STR:
        ns << "switch ( HPIPE_PARSE_FUNC_NAME( data ) ) {";
        ns << "case RET_CONT: if ( os.str().size() ) os << ' '; os << \"status=CNT\"; break;"; //
        ns << "case RET_OK  : if ( os.str().size() ) os << ' '; os << \"status=OK\" ; break;";
        ns << "case RET_KO  : if ( os.str().size() ) os << ' '; os << \"status=KO\" ; break;";
        ns << "}";
        break;
    }
    ss << "    std::cout << \"  \" << name << \" -> \" << ( os.str() == expected ? \"(OK) \" : \"(BAD) \" ) << os.str() << std::endl;";
    ss << "    return os.str() != expected;";
    ss << "}";
    ss << "";
    write_definitions( ss );

    ss << "int main() {";
    for( const Lexer::TestData &td : tds ) {
        *ss.stream << "    static unsigned char name_" << &td << "[] = { "; for( char c : td.name ) *ss.stream << (int)c << ","; *ss.stream << " 0 };\n";
        *ss.stream << "    static unsigned char inp_"  << &td << "[] = { "; for( char c : td.inp  ) *ss.stream << (int)c << ","; *ss.stream << " 0 };\n";
        *ss.stream << "    static unsigned char out_"  << &td << "[] = { "; for( char c : td.out  ) *ss.stream << (int)c << ","; *ss.stream << " 0 };\n";
    }
    ss << "    int res = 0;";
    for( const Lexer::TestData &td : tds ) {
        if ( need_buf() )
            ss << "    Hpipe::Buffer::nb_alive_bufs = 0;";
        ss << "    res |= Test().exec( name_" << &td << ", inp_" << &td << ", " << td.inp.size() << ", std::string( (const char *)out_" << &td << ", " << td.out.size() << " ) );";
        if ( need_buf() )
            ss << "    if ( Hpipe::Buffer::nb_alive_bufs ) { std::cout << \"(BAD) nb_remaining_bufs=\" << Hpipe::Buffer::nb_alive_bufs << std::endl; res = 1; }";
    }
    ss << "    return res;";
    ss << "}";
    fout.close();

    return system( "g++ -DINCLUDE_CPP -g3 -std=c++14 -Isrc/ -o out out.cpp && ./out" );
}

bool CppEmitter::bench(const std::vector<Lexer::TrainingData> &tds) {
    std::ofstream fout( "bench.cpp" );
    StreamSepMaker ss( fout );
    StreamSepMaker ns( fout, "    " );
    ss << "#include <Hpipe/CbStringPtr.h>";
    ss << "#include <Hpipe/Print.h>";
    ss << "#include <iostream>";
    ss << "#include <sstream>";
    ss << "#include <ctime>";    
    ss << "";
    ss << "#define HPIPE_DEFINITION_PREFIX Bench::";
    ss << "";
    write_preliminaries( ss );
    ss << "";
    ss << "struct Bench {";
    ss << "    void exec( const unsigned char *name, const unsigned char *data, unsigned size, const char *display );";
    write_declarations( ns );
    ss << "    std::ostringstream os;";
    ss << "    HpipeData hpipe_data;";
    ss << "    unsigned cpt = 0;";
    ss << "};";

    ss << "void Bench::exec( const unsigned char *name, const unsigned char *data, unsigned size, const char *display ) {";
    switch ( buffer_type ) {
    case BT_HPIPE_CB_STRING_PTR:
        ns << "Hpipe::Buffer *buf = Hpipe::Buffer::New( size );";
        ns << "memcpy( buf->data, data, size );";
        ns << "buf->used = size;";
        ns << "auto t0 = std::clock();";
        ns << "for( unsigned var = 0; var < 1000000; ++var ) {";
        ns << "    Bench::HPIPE_DATA_CTOR_NAME( &hpipe_data );";
        ns << "    parse( buf, 0, size );";
        ns << "}";
        ns << "auto t1 = std::clock();";
        ns << "HPIPE_BUFF_T::dec_ref( buf );";
        break;
    case BT_HPIPE_BUFFER:
        ns << "Hpipe::Buffer *buf = Hpipe::Buffer::New( size );";
        ns << "memcpy( buf->data, data, size );";
        ns << "buf->used = size;";
        ns << "auto t0 = std::clock();";
        ns << "for( unsigned var = 0; var < 1000000; ++var ) {";
        ns << "    Bench::HPIPE_DATA_CTOR_NAME( &hpipe_data );";
        ns << "    parse( buf, true );";
        ns << "}";
        ns << "auto t1 = std::clock();";
        ns << "HPIPE_BUFF_T::dec_ref( buf );";
        break;
    case BT_BEG_END:
        ns << "auto t0 = std::clock();";
        ns << "for( unsigned var = 0; var < 1000000; ++var ) {";
        ns << "    Bench::HPIPE_DATA_CTOR_NAME( &hpipe_data );";
        ns << "    parse( data, data - 1 + size );";
        ns << "}";
        ns << "auto t1 = std::clock();";
        break;
    case BT_C_STR:
        ns << "auto t0 = std::clock();";
        ns << "for( unsigned var = 0; var < 1000000; ++var ) {";
        ns << "    Bench::HPIPE_DATA_CTOR_NAME( &hpipe_data );";
        ns << "    parse( data );";
        ns << "}";
        ns << "auto t1 = std::clock();";
        break;
    }
    ss << "    if ( display )";
    ss << "        std::cout << name << \", \" << display << \" -> \" << double( t1 - t0 ) / CLOCKS_PER_SEC << std::endl;";
    ss << "}";

    write_definitions( ss );

    ss << "int main( int argc, char **argv ) {";
    for( const Lexer::TrainingData &td : tds ) {
        *ss.stream << "    static unsigned char name_" << &td << "[] = { "; for( char c : td.name ) *ss.stream << (int)c << ","; *ss.stream << " 0 };\n";
        *ss.stream << "    static unsigned char inp_"  << &td << "[] = { "; for( char c : td.inp  ) *ss.stream << (int)c << ","; *ss.stream << " 0 };\n";
        *ss.stream << "    static double freq_"        << &td << " = " << td.freq << ";\n";
    }
    for( const Lexer::TrainingData &td : tds )
        ss << "    Bench().exec( name_" << &td << ", inp_" << &td << ", " << td.inp.size() << ", argc > 1 ? argv[ 1 ] : 0 );";
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

bool CppEmitter::interruptible() const {
    return buffer_type == BT_HPIPE_BUFFER;
}

bool CppEmitter::need_buf() const {
    return buffer_type == BT_HPIPE_BUFFER || buffer_type == BT_HPIPE_CB_STRING_PTR;
}

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

void CppEmitter::write_label( StreamSepMaker &ss, unsigned num, char letter ) {
    ss.rm_beg( 2 ) << letter << "_" << num << ":" << ( trace_labels ? " std::cout << \"" + to_string( letter ) + "_" + to_string( num ) + "\\t\" << __LINE__ << std::endl;" : "" );
}

void CppEmitter::add_variable(const std::string &name, const std::string &type, const std::string &default_value ) {
    if ( variables.count( name ) )
        return;
    variables[ name ] = { type, default_value };
}

} // namespace Hpipe

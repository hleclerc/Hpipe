#include "InstructionCode.h"
#include "InstructionNone.h"
#include "InstructionSave.h"
#include "FindVarInCode.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionCode::InstructionCode( const Context &cx, const std::string &code, int num_active_item ) : InstructionWithCode( cx, num_active_item ), code( code ) {
}

void InstructionCode::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << ( code.size() > 12 ? code.substr( 0, 9 ) + "..." : code );
}

Instruction *InstructionCode::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    int ind = ncx.pos.index_first( cx.pos[ num_active_item ] );
    if ( ind >= 0 )
        return inst_pool << new InstructionCode( ncx, code, ind );
    return inst_pool << new InstructionNone( ncx );
}

bool InstructionCode::has_ret_cont() const {
    return find_var_in_code( code, "RETURN_STOP_CONT", 0 ) != std::string::npos;
}

void InstructionCode::write_code( StreamSepMaker &ss, CppEmitter *cpp_emitter, std::string str, const std::string &repl ) {
    // variables
    for( auto &p : cpp_emitter->variables ) {
        for( std::string::size_type pos = 0; ; ) {
            pos = find_var_in_code( str, p.first, pos );
            if ( pos == std::string::npos )
                break;
            str = str.replace( pos, 0, "sipe_data->" );
            pos += 11 + p.first.size();
        }
    }

    // RETURN_CONT
    for( std::string::size_type pos = 0; ; ) {
        pos = find_var_in_code( str, "RETURN_STOP_CONT", pos );
        if ( pos == std::string::npos )
            break;
        // ++cpp_emitter->nb_cont_label;
        std::ostringstream val;
        val << "sipe_data->inp_cont = &&c_" << cpp_emitter->nb_cont_label << "; return RET_STOP_CONT; c_" << ++cpp_emitter->nb_cont_label << ":;";
        str = str.replace( pos, 16, val.str() );
        pos += val.str().size();
    }

    // data -> *( save + x ), ...
    if ( repl.size() )
        str = cpp_emitter->repl_data( str, repl );

    // __data -> data
    for( auto &p : cpp_emitter->variables ) {
        for( std::string::size_type pos = 0; ; ) {
            pos = find_var_in_code( str, "__data", pos );
            if ( pos == std::string::npos )
                break;
            str = str.replace( pos, 2, "", 0 );
            pos += 4;
        }
    }

    ss << str;
}

void InstructionCode::get_code_repr( std::ostream &os ) {
    os << "CODE " << code.size() << " " << code;
}

void InstructionCode::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    write_code( ss, cpp_emitter, code );
    write_trans( ss, cpp_emitter );
}

void InstructionCode::write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter, std::string repl_data ) {
    if ( save )
        repl_data = "( " + std::string( cpp_emitter->rewind_rec_level or not cpp_emitter->interruptible() ? "" : "sipe_data->__" ) + "save + " + to_string( save->num_save ) + " )";

    write_code( ss, cpp_emitter, code, repl_data );
}

bool InstructionCode::data_code() const {
    return find_var_in_code( code, "data", 0 ) != std::string::npos;
}

} // namespace Hpipe

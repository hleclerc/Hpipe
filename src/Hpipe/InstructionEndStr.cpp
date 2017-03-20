#include "InstructionEndStr.h"
#include "InstructionNone.h"
#include "InstructionSave.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionEndStr::InstructionEndStr( const Context &cx, const std::string &var, int num_active_item, bool want_next_char ) : InstructionWithCode( cx, num_active_item ), var( var ), want_next_char( want_next_char ) {
    running_strs.erase( var );
}

void InstructionEndStr::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "END_STR(" << var << "," << want_next_char << ")";
}

Instruction *InstructionEndStr::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    int ind = ncx.pos.index_first( cx.pos[ num_active_item ] );
    if ( ind >= 0 )
        return inst_pool << new InstructionEndStr( ncx, var, ind, want_next_char );
    return inst_pool << new InstructionNone( ncx );
}

void InstructionEndStr::get_code_repr( std::ostream &os ) {
    os << "END_STR " << var.size() << " " << var << " " << want_next_char;
}

void InstructionEndStr::update_running_strings( std::set<std::string> &strs ) const {
    strs.erase( var );
}

bool InstructionEndStr::works_on_next() const {
    return want_next_char;
}

void InstructionEndStr::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    write_cpp_code_seq( ss, es, cpp_emitter );
    write_trans( ss, cpp_emitter );
}

void InstructionEndStr::write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter, std::string repl_data, std::string repl_buf ) {
    if ( cpp_emitter->buffer_type == CppEmitter::BT_HPIPE_BUFFER ) {
        cpp_emitter->preliminaries.push_back_unique( "#ifndef HPIPE_CB_STRING__ASSIGN_BEG_END\n#define HPIPE_CB_STRING__ASSIGN_BEG_END( var, beg_buf, beg_ptr, end_buf, end_ptr ) var = HPIPE_CB_STRING_T( HPIPE_CB_STRING_T::NoIncRef(), beg_buf, beg_ptr, end_buf, end_ptr )\n#endif // HPIPE_CB_STRING__ASSIGN_BEG_END\n" );

        if ( want_next_char ) {
            ss << "HPIPE_BUFF_T__INC_REF( " << repl_buf << " );";
            ss << "HPIPE_BUFF_T__SKIP( HPIPE_DATA.__beg_" << var << "_buf, HPIPE_DATA.__beg_" << var << "_data, HPIPE_DATA.__beg_" << var << "_off );";
            ss << "HPIPE_CB_STRING__ASSIGN_BEG_END( HPIPE_DATA." << var << ", HPIPE_DATA.__beg_" << var << "_buf, HPIPE_DATA.__beg_" << var << "_data, " << repl_buf << ", " << repl_data << " + 1 );";
        } else {
            ss << "if ( " << repl_data << " > " << repl_buf << "->data ) HPIPE_BUFF_T__INC_REF( " << repl_buf << " );";
            ss << "HPIPE_BUFF_T__SKIP( HPIPE_DATA.__beg_" << var << "_buf, HPIPE_DATA.__beg_" << var << "_data, HPIPE_DATA.__beg_" << var << "_off );";
            ss << "HPIPE_CB_STRING__ASSIGN_BEG_END( HPIPE_DATA." << var << ", HPIPE_DATA.__beg_" << var << "_buf, HPIPE_DATA.__beg_" << var << "_data, " << repl_buf << ", " << repl_data << " );";
        }
    } else {
        cpp_emitter->preliminaries.push_back_unique( "#ifndef HPIPE_CM_STRING__ASSIGN_BEG_END\n#define HPIPE_CM_STRING__ASSIGN_BEG_END( var, beg_ptr, end_ptr ) var = HPIPE_CM_STRING( beg_ptr, end_ptr )\n#endif // HPIPE_CM_STRING__ASSIGN_BEG_END\n" );

        if ( want_next_char ) {
            ss << "HPIPE_CM_STRING__ASSIGN_BEG_END( HPIPE_DATA." << var << ", HPIPE_DATA.__beg_" << var << "_data, " << repl_data << " + 1 );";
        } else {
            ss << "HPIPE_CM_STRING__ASSIGN_BEG_END( HPIPE_DATA." << var << ", HPIPE_DATA.__beg_" << var << "_data, " << repl_data << " );";
        }
    }
}

bool InstructionEndStr::data_code() const {
    return true;
}

InstructionWithCode *InstructionEndStr::no_works_on_next_clone( PtrPool<Instruction> &inst_pool ) const {
    return inst_pool << new InstructionEndStr( cx, var, num_active_item, false );
}


} // namespace Hpipe

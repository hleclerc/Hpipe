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

void InstructionEndStr::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    write_cpp_code_seq( ss, es, cpp_emitter );
    write_trans( ss, cpp_emitter );
}

void InstructionEndStr::write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter, std::string repl_data, std::string repl_buf ) {
    if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER ) {
        ss << "HPIPE_BUFFER::skip( sipe_data->__beg_" << var << "_buf, sipe_data->__beg_" << var << "_data, sipe_data->__beg_" << var << "_off );";
        if ( want_next_char ) {
            ss << "HPIPE_BUFFER::inc_ref( " << repl_buf << " );";
            ss << "sipe_data->" << var << " = Hpipe::CbString{ Hpipe::CbString::NoIncRef(), sipe_data->__beg_" << var << "_buf, sipe_data->__beg_" << var << "_data, " << repl_buf << ", " << repl_data << " + 1 };";
        } else {
            ss << "if ( " << repl_data << " > " << repl_buf << "->data ) HPIPE_BUFFER::inc_ref( " << repl_buf << " );";
            ss << "sipe_data->" << var << " = Hpipe::CbString{ Hpipe::CbString::NoIncRef(), sipe_data->__beg_" << var << "_buf, sipe_data->__beg_" << var << "_data, " << repl_buf << ", " << repl_data << " };";
        }
    } else {
        if ( want_next_char ) {
            ss << "sipe_data->" << var << " = Hpipe::CmString{ sipe_data->__beg_" << var << "_data, " << repl_data << " + 1 };";
        } else {
            ss << "sipe_data->" << var << " = Hpipe::CmString{ sipe_data->__beg_" << var << "_data, " << repl_data << " };";
        }
    }
}

bool InstructionEndStr::data_code() const {
    return true;
}

} // namespace Hpipe

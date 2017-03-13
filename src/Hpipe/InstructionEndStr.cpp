#include "InstructionEndStr.h"
#include "InstructionNone.h"
#include "InstructionSave.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionEndStr::InstructionEndStr( const Context &cx, const std::string &var, int num_active_item ) : InstructionWithCode( cx, num_active_item ), var( var ) {
}

void InstructionEndStr::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "END_STR(" << var << ")";
}

Instruction *InstructionEndStr::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    int ind = ncx.pos.index_first( cx.pos[ num_active_item ] );
    if ( ind >= 0 )
        return inst_pool << new InstructionEndStr( ncx, var, ind );
    return inst_pool << new InstructionNone( ncx );
}

void InstructionEndStr::get_code_repr( std::ostream &os ) {
    os << "END_STR " << var.size() << " " << var;
}

void InstructionEndStr::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    write_cpp_code_seq( ss, es, cpp_emitter );
    write_trans( ss, cpp_emitter );
}

void InstructionEndStr::write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter, std::string repl_data, std::string repl_buf ) {
    if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER )
        ss << "sipe_data->" << var << " = Hpipe::CbString{ sipe_data->__beg_" << var << "_buf, sipe_data->__beg_" << var << "_data, " << repl_buf << ", " << repl_data << " + 1 };";
    else
        HPIPE_TODO;
}

bool InstructionEndStr::data_code() const {
    return true;
}

} // namespace Hpipe

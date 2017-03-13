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

void InstructionEndStr::write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter, std::string repl_data ) {
    ss << "sipe_data->" << var << " = { __beg_" << var << "_buf, __beg_" << var << "_data, buf, data };";
}

bool InstructionEndStr::data_code() const {
    return true;
}

} // namespace Hpipe

#include "InstructionClrStr.h"
#include "InstructionNone.h"

namespace Hpipe {

InstructionClrStr::InstructionClrStr( const Context &cx, const std::string &var, int num_active_item ) : InstructionWithCode( cx, num_active_item ), var( var ) {
}

void InstructionClrStr::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "CS(" << var << ")";
}

Instruction *InstructionClrStr::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    int ind = ncx.pos.index_first( cx.pos[ num_active_item ] );
    if ( ind >= 0 )
        return inst_pool << new InstructionClrStr( ncx, var, ind );
    return inst_pool << new InstructionNone( ncx );
}

void InstructionClrStr::get_code_repr( std::ostream &os ) {
    os << "CLR_STR " << var.size() << " " << var;
}

void InstructionClrStr::reg_var( std::function<void (std::string, std::string)> f ) {
    f( "std::string", var );
}

void InstructionClrStr::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    ss << "sipe_data->" << var << ".clear();";
    write_trans( ss, cpp_emitter );
}

void InstructionClrStr::write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter, std::string repl_data ) {
    ss << "sipe_data->" << var << ".clear();";
}

} // namespace Hpipe

#include "InstructionSave.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionSave::InstructionSave( const Context &cx, unsigned num_save ) : Instruction( cx ), num_save( num_save ) {

}

void InstructionSave::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "S(" << num_save << ")";
}

Instruction *InstructionSave::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    return inst_pool << new InstructionSave( ncx, num_save );
}

int InstructionSave::save_in_loc_reg() const {
    return num_save;
}

void InstructionSave::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    ss << ( ! cpp_emitter->interruptible() ? "" : "HPIPE_DATA.__" ) << "save[ " << num_save << " ] = *data;";
    write_trans( ss, cpp_emitter );
}

void InstructionSave::get_code_repr( std::ostream &os ) {
    os << "SAVE " << num_save;
}


} // namespace Hpipe

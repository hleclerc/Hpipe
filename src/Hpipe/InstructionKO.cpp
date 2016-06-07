#include "InstructionKO.h"

namespace Hpipe {

void InstructionKO::write_dot( std::ostream &os ) const {
    os << "KO";
}

Instruction *InstructionKO::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    return inst_pool << new InstructionKO( ncx );
}

void InstructionKO::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    ss << "return RET_KO;";
}


} // namespace Hpipe

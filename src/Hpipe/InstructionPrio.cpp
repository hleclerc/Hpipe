#include "InstructionPrio.h"

namespace Hpipe {

void InstructionPrio::write_dot( std::ostream &os ) const {
    os << "PR";
}

Instruction *InstructionPrio::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    return inst_pool << new InstructionPrio( ncx );
}

bool InstructionPrio::can_be_deleted() const {
    return true;
}

} // namespace Hpipe

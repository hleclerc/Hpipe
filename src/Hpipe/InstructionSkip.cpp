#include "InstructionSkip.h"

namespace Hpipe {

void InstructionSkip::write_dot( std::ostream &os ) const {
    os << "SK";
}

Instruction *InstructionSkip::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    return inst_pool << new InstructionSkip( ncx );
}

bool InstructionSkip::can_be_deleted() const {
    return true;
}

} // namespace Hpipe

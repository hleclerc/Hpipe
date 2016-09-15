#include "InstructionNone.h"

namespace Hpipe {

void InstructionNone::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "NONE";
}

Instruction *InstructionNone::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    return inst_pool << new InstructionNone( ncx );
}

bool InstructionNone::can_be_deleted() const {
    return true;
}

} // namespace Hpipe

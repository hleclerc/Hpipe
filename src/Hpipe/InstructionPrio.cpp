#include "InstructionPrio.h"

namespace Hpipe {

void InstructionPrio::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "PR";
}

Instruction *InstructionPrio::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    return inst_pool << new InstructionPrio( ncx );
}

void InstructionPrio::get_code_repr( std::ostream &os ) {
    os << "PRIO";
}

bool InstructionPrio::can_be_deleted() const {
    return true;
}

} // namespace Hpipe

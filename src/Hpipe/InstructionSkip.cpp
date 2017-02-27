#include "InstructionSkip.h"

namespace Hpipe {

void InstructionSkip::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "SK";
}

Instruction *InstructionSkip::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    return inst_pool << new InstructionSkip( ncx );
}

void InstructionSkip::get_code_repr( std::ostream &os ) {
    os << "SKIP";
}

bool InstructionSkip::can_be_deleted() const {
    return true;
}

} // namespace Hpipe

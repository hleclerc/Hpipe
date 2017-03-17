#include "InstructionWithCode.h"

namespace Hpipe {

InstructionWithCode::InstructionWithCode( const Context &cx, int num_active_item ) : Instruction( cx ), num_active_item( num_active_item ) {
    in_code_seq = false;
    save = 0;
}

bool InstructionWithCode::with_code() const {
    return true;
}

bool InstructionWithCode::can_be_deleted() const {
    return mark; // cx.pos.size() > 1;
}

void InstructionWithCode::update_running_strings( std::set<std::string> &strs ) const {
}

bool InstructionWithCode::works_on_next() const {
    return false;
}

InstructionWithCode *InstructionWithCode::no_works_on_next_clone( PtrPool<Instruction> &inst_pool ) const {
    return 0;
}


} // namespace Hpipe

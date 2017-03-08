#include "InstructionWithCode.h"

namespace Hpipe {

InstructionWithCode::InstructionWithCode( const Context &cx, int num_active_item ) : Instruction( cx ), num_active_item( num_active_item ) {
    save = 0;
}

bool InstructionWithCode::with_code() const {
    return true;
}

bool InstructionWithCode::can_be_deleted() const {
    return false; // mark; // cx.pos.size() > 1;
}


} // namespace Hpipe

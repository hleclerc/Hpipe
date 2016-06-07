#include "InstructionWithCode.h"

namespace Hpipe {

InstructionWithCode::InstructionWithCode( const Context &cx, const CharItem *active_ci ) : Instruction( cx ), active_ci( active_ci ) {
    save = 0;
}

bool InstructionWithCode::with_code() const {
    return true;
}

bool InstructionWithCode::can_be_deleted() const {
    return mark; // cx.pos.size() > 1;
}


} // namespace Hpipe

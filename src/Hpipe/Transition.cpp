#include "Instruction.h"

namespace Hpipe {

Transition::Transition( Instruction *inst, const Vec<unsigned> &rcitem, double freq ) : inst( inst ), rcitem( rcitem ), freq( freq ) {
    freq = -1;
}

void Transition::write_to_stream( std::ostream &os ) const {
    os << inst->get_display_id();
}


} // namespace Hpipe

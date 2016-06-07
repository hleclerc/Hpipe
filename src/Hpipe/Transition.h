#pragma once

#include "Vec.h"

namespace Hpipe {
class Instruction;

/**
*/
class Transition {
public:
    Transition( Instruction *inst = 0, const Vec<unsigned> &rcitem = {}, double freq = -1.0 );

    void          write_to_stream( std::ostream &os ) const;

    Instruction  *inst;
    Vec<unsigned> rcitem;

    double        freq;
};

} // namespace Hpipe

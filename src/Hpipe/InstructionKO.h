#pragma once

#include "Instruction.h"

namespace Hpipe {

/**
*/
class InstructionKO : public Instruction {
public:
    using Instruction::Instruction;
    virtual void         write_dot( std::ostream &os ) const;
    virtual void         write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual Instruction *clone    ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
};

} // namespace Hpipe

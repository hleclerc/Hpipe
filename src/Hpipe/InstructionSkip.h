#pragma once

#include "Instruction.h"

namespace Hpipe {

/**
  Skipped CharItem (e.g. BEGIN, LABEL, ...)
*/
class InstructionSkip : public Instruction {
public:
    using Instruction::Instruction;
    virtual void         write_dot     ( std::ostream &os, std::vector<std::string> *edge_labels = 0 ) const;
    virtual bool         can_be_deleted() const;
    virtual Instruction *clone         ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
};

} // namespace Hpipe

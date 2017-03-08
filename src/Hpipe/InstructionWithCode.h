#pragma once

#include "Instruction.h"

namespace Hpipe {
class InstructionSave;


/**
*/
class InstructionWithCode : public Instruction {
public:
    InstructionWithCode( const Context &cx, int num_active_item );

    virtual bool     with_code     () const;
    virtual bool     can_be_deleted() const;

    int              num_active_item; ///< active CharItem in cx
    InstructionSave *save;      ///<
};

} // namespace Hpipe

#pragma once

#include "Instruction.h"

namespace Hpipe {
class InstructionSave;


/**
*/
class InstructionWithCode : public Instruction {
public:
    InstructionWithCode( const Context &cx, const CharItem *active_ci );

    virtual bool     with_code     () const;
    virtual bool     can_be_deleted() const;

    const CharItem  *active_ci; ///< num of active CharItem in cx
    InstructionSave *save;      ///<
};

} // namespace Hpipe

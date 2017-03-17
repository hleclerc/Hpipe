#pragma once

#include "Instruction.h"

namespace Hpipe {
class InstructionSave;


/**
*/
class InstructionWithCode : public Instruction {
public:
    InstructionWithCode( const Context &cx, int num_active_item );

    virtual bool                 with_code             () const;
    virtual bool                 can_be_deleted        () const;
    virtual void                 update_running_strings( std::set<std::string> &strs ) const;
    virtual bool                 works_on_next         () const;
    virtual InstructionWithCode *no_works_on_next_clone( PtrPool<Instruction> &inst_pool ) const;

    int                          num_active_item; ///< active CharItem in cx
    InstructionSave             *save;            ///<
    bool                         in_code_seq;     ///>
};

} // namespace Hpipe

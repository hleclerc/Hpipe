#pragma once

#include "Instruction.h"

namespace Hpipe {

/**
*/
class InstructionSave : public Instruction {
public:
    InstructionSave( const Context &cx, unsigned num_save );

    virtual void         write_dot      ( std::ostream &os ) const;
    virtual Instruction *clone          ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual int          save_in_loc_reg() const;
    virtual void         write_cpp      ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );

    unsigned             num_save;
};

} // namespace Hpipe

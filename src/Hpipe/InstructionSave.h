#pragma once

#include "Instruction.h"

namespace Hpipe {

/**
*/
class InstructionSave : public Instruction {
public:
    InstructionSave( const Context &cx, unsigned num_save );

    virtual void         write_dot      ( std::ostream &os, std::vector<std::string> *edge_labels = 0 ) const;
    virtual Instruction *clone          ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual int          save_in_loc_reg() const;
    virtual void         write_cpp      ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual void         get_code_repr  ( std::ostream &os ) override;

    unsigned             num_save;
};

} // namespace Hpipe

#pragma once

#include "Instruction.h"

namespace Hpipe {

/**
*/
class InstructionOK : public Instruction {
public:
    using Instruction::Instruction;
    virtual void         write_dot    ( std::ostream &os, std::vector<std::string> *edge_labels = 0 ) const;
    virtual void         write_cpp    ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual Instruction *clone        ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual void         get_code_repr( std::ostream &os ) override {
        os << "OK";
    }
};

} // namespace Hpipe

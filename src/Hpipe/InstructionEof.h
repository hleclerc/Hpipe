#pragma once

#include "Instruction.h"

namespace Hpipe {

/**
*/
class InstructionEof : public Instruction {
public:
    InstructionEof( const Context &cx, bool beg );

    virtual void         write_dot( std::ostream &os ) const;
    virtual Instruction *clone    ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual void         write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );

    bool                 beg;
};

} // namespace Hpipe

#pragma once

#include "Instruction.h"

namespace Hpipe {
class InstructionRewind;

/**
*/
class InstructionMark : public Instruction {
public:
    InstructionMark( const Context &cx, unsigned num_active_item );

    virtual void             write_dot( std::ostream &os ) const;
    virtual Instruction     *clone    ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual bool             is_a_mark() const;
    virtual void             write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual Transition      *train    ( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous );
    virtual bool             with_code() const;

    Vec<InstructionRewind *> rewinds;
    unsigned                 num_active_item;
};

} // namespace Hpipe

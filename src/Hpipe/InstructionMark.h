#pragma once

#include "Instruction.h"

namespace Hpipe {
class InstructionRewind;

/**
*/
class InstructionMark : public Instruction {
public:
    InstructionMark( const Context &cx, unsigned num_active_item );

    virtual void             write_dot    ( std::ostream &os, std::vector<std::string> *edge_labels = 0 ) const;
    virtual Instruction     *clone        ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual void             write_cpp    ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual Transition      *train        ( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous );
    virtual bool             with_code    () const;
    virtual void             get_code_repr( std::ostream &os ) override;

    Vec<InstructionRewind *> rewinds;
    unsigned                 num_active_item; ///< index in cx.pos of item that caused the mark
};

} // namespace Hpipe

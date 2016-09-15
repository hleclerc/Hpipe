#pragma once

#include "Instruction.h"

namespace Hpipe {

/**
*/
class InstructionNextChar : public Instruction {
public:
    InstructionNextChar( const Context &cx, bool beg, bool assume_not_eof );

    virtual void         write_dot           ( std::ostream &os, std::vector<std::string> *edge_labels = 0 ) const;
    virtual Transition  *train               ( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous );
    virtual void         write_cpp           ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual Instruction *clone               ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual void         boyer_moore_opt     ( PtrPool<Instruction> &inst_pool, Instruction **init );
    Instruction         *make_boyer_moore_rec( PtrPool<Instruction> &inst_pool, const Vec<std::pair<Vec<Cond>,Instruction *>> &front, InstructionNextChar *next_char, int orig_front_size );

    bool                 beg;
    bool                 assume_not_eof;
};

} // namespace Hpipe

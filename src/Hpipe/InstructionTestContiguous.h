#pragma once

#include "Instruction.h"

namespace Hpipe {

/**
*/
class InstructionTestContiguous : public Instruction {
public:
    InstructionTestContiguous( const Context &cx, bool beg, unsigned nb_chars );

    virtual void         write_dot  ( std::ostream &os, std::vector<std::string> *edge_labels = 0 ) const;
    virtual Transition  *train      ( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous );
    virtual Instruction *clone      ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual void         write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );

    bool                 beg;
    unsigned             nb_chars;
};

} // namespace Hpipe

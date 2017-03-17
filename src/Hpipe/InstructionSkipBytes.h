#pragma once

#include "Instruction.h"

namespace Hpipe {

/**
*/
class InstructionSkipBytes : public Instruction {
public:
    InstructionSkipBytes( const Context &cx, const std::string &var, bool beg );

    virtual void         write_dot( std::ostream &os, std::vector<std::string> *edge_labels = 0 ) const;
    virtual Instruction *clone    ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual void         write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual Transition  *train    ( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous );
    virtual void         get_code_repr( std::ostream &os ) override;

    std::string          var;
    bool                 beg;
};

} // namespace Hpipe

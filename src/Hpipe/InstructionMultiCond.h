#pragma once

#include "Instruction.h"

namespace Hpipe {

/**
*/
class InstructionMultiCond : public Instruction {
public:
    InstructionMultiCond( const Context &cx, const Vec<Cond> &conds, int off_data = 0 );

    virtual void         write_dot          ( std::ostream &os, std::vector<std::string> *edge_labels = 0 ) const;
    virtual Instruction *clone              ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual Transition  *train              ( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous );
    virtual void         write_cpp          ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual void         optimize_conditions( PtrPool<Instruction> &inst_pool );
    virtual void         merge_eq_next      ( PtrPool<Instruction> &inst_pool );
    virtual bool         can_be_deleted     () const;
    virtual void         get_code_repr      ( std::ostream &os ) override;

    Vec<Cond> conds;
    int       off_data;
};

} // namespace Hpipe

#pragma once

#include "Instruction.h"
#include "BranchSet.h"
#include "Cond.h"

namespace Hpipe {

/**
*/
class InstructionCond : public Instruction {
public:
    InstructionCond( const Context &cx, const Cond &cond, int off_data = 0, const Cond &not_in = {} );

    virtual void         write_dot          ( std::ostream &os ) const;
    virtual Instruction *clone              ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual Transition  *train              ( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous );
    virtual void         write_cpp          ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual void         optimize_conditions( PtrPool<Instruction> &inst_pool );
    virtual void         find_cond_leaves   ( std::map<Instruction *,Cond> &leaves, const Cond &in, Instruction *orig );
    virtual bool         same_code          ( const Instruction *that ) const;

    static Instruction  *make_cond          ( const BranchSet::Node *node, PtrPool<Instruction> &inst_pool, const Cond &not_in, bool in_a_cycle, InstructionMark *mark, int off_data );

    Cond cond;
    Cond not_in;
    int  off_data;
};

} // namespace Hpipe

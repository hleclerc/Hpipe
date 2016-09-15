#pragma once

#include "Instruction.h"

namespace Hpipe {
class InstructionWithCode;
class InstructionMark;

/**
*/
class InstructionRewind : public Instruction {
public:
    InstructionRewind( const Context &cx );

    virtual void               write_dot          ( std::ostream &os, std::vector<std::string> *edge_labels = 0 ) const;
    virtual void               write_dot_add      ( std::ostream &os, bool disp_inst_pred, bool disp_trans_freq, bool disp_rc_item ) const;
    virtual Instruction       *clone              ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual void               apply_rec          ( std::function<void(Instruction *)> f, bool subgraphs = false );
    virtual void               get_unused_rec     ( Vec<Instruction *> &to_remove, Instruction *&init );
    virtual void               apply_rec_rewind_l ( std::function<void(Instruction *, unsigned)> f, unsigned rewind_level = 0 );
    bool                       use_exec           () const;
    bool                       use_code_seq       () const;
    virtual bool               with_code          () const;
    virtual void               write_cpp          ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual bool               merge_predecessors ( Instruction **init );
    virtual bool               same_code          ( const Instruction *that ) const;
    virtual void               optimize_conditions( PtrPool<Instruction> &inst_pool );
    virtual Transition        *train              ( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous );
    virtual void               reg_var            ( std::function<void (std::string, std::string)> f );


    Instruction               *exec;
    Vec<InstructionWithCode *> code_seq;
    bool                       has_code = false;
    bool                       has_data_code = false;
    bool                       has_code_in_a_cycle = false;
};

} // namespace Hpipe

#pragma once

#include "Instruction.h"

namespace Hpipe {
class InstructionWithCode;
class InstructionMark;

/**
*/
class InstructionRewind : public Instruction {
public:
    struct CodeSeqItem {
        CodeSeqItem( unsigned offset, InstructionWithCode *code ) : offset( offset ), code( code ) {}
        int                  offset; ///< -1 means that code does not need data
        InstructionWithCode *code;
    };
    enum {
        USE_NCX_NONE,
        USE_NCX_BEG,
        USE_NCX_END,
    };
    InstructionRewind( const Context &cx );

    virtual void         write_dot           ( std::ostream &os, std::vector<std::string> *edge_labels = 0 ) const;
    virtual void         write_dot_add       ( std::ostream &os, bool disp_inst_pred, bool disp_trans_freq, bool disp_rc_item ) const;
    virtual Instruction *clone               ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual void         apply_rec           ( std::function<void(Instruction *)> f, bool subgraphs = false );
    virtual void         get_unused_rec      ( Vec<Instruction *> &to_remove, Instruction *&init );
    virtual void         apply_rec_rewind_l  ( std::function<void(Instruction *, unsigned)> f, unsigned rewind_level = 0 );
    virtual bool         with_code           () const;
    virtual void         write_cpp           ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual Transition  *train               ( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous );
    virtual void         reg_var             ( std::function<void (std::string, std::string)> f, CppEmitter *cpp_emitter );
    virtual void         get_code_repr       ( std::ostream &os ) override;

    bool                 use_data_in_code_seq() const;
    bool                 no_code_at_all      () const;

    Instruction         *exec;                     ///< intermediate representation. Used to update the following attibutes
    Vec<CodeSeqItem>     code_seq_beg;             ///< code from mark
    Vec<CodeSeqItem>     code_seq_end;             ///< code from rewind
    Vec<std::string>     strs_to_cancel;           ///< strings to cancel before the mark

    unsigned             offset_for_ncx = 0;
    int                  use_of_ncx = USE_NCX_BEG; ///< USE_NCX_BEG, ...
    Context::PC          ncx;                      ///< new context

    bool                 need_rw        = false;   ///< true if needed to rewind at mark (+ offset_for_ncx)
};

} // namespace Hpipe

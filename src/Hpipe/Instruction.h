#pragma once

#include "Transition.h"
#include "CharItem.h"
#include "Context.h"
#include "PtrPool.h"
#include <map>

namespace Hpipe {
class InstructionMark;
class CppEmitter;

/**
*/
class Instruction {
public:
    Instruction( Context cx );
    virtual ~Instruction();

    virtual void         write_to_stream    ( std::ostream &os ) const;
    virtual void         write_dot          ( std::ostream &os, std::vector<std::string> *edge_labels = 0 ) const = 0;
    virtual void         write_dot_add      ( std::ostream &os, bool disp_inst_pred, bool disp_trans_freq, bool disp_rc_item ) const;
    void                 write_dot_rec      ( std::ostream &os, bool disp_inst_pred, bool disp_trans_freq, bool disp_rc_item ) const;
    virtual Instruction *clone              ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) = 0;
    unsigned             get_display_id     () const;
    void                 apply              ( std::function<void(Instruction *)> f, bool subgraphs = false );
    virtual void         apply_rec          ( std::function<void(Instruction *)> f, bool subgraphs = false );
    virtual void         get_unused_rec     ( Vec<Instruction *> &to_remove, Instruction *&init );
    bool                 find_rec           ( std::function<bool(Instruction *)> f );
    virtual void         apply_rec_rewind_l ( std::function<void(Instruction *, unsigned)> f, unsigned rewind_level = 0 );
    virtual Transition  *train              ( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous );
    void                 train_rec          ( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous );
    virtual bool         can_be_deleted     () const;
    virtual bool         is_a_mark          () const;
    virtual int          save_in_loc_reg    () const;
    void                 remove             ( bool update_rcitem = true ); ///< remove from graph
    void                 insert_before_this ( Instruction *inst, Instruction *&init ); ///< inst -> this
    void                 repl_in_preds      ( Instruction *inst ); ///< replace in graph
    virtual void         reg_var            ( std::function<void( std::string type, std::string name )> f );
    virtual void         write_cpp          ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    int                  get_id_gen         ( CppEmitter *cpp_emitter );
    void                 write_trans        ( StreamSepMaker &ss, CppEmitter *cpp_emitter, unsigned num = 0 );
    virtual void         optimize_conditions( PtrPool<Instruction> &inst_pool );
    virtual void         find_cond_leaves   ( std::map<Instruction *, Cond> &leaves, const Cond &in, Instruction *orig );
    virtual bool         with_code          () const;
    virtual bool         data_code          () const;
    virtual bool         has_ret_cont       () const;
    virtual bool         always_need_id_gen ( CppEmitter *cpp_emitter ) const;
    void                 update_in_a_branch ();
    void                 update_in_a_cycle  ();
    // void              get_boyer_moore_seq( Vec<std::pair<Vec<Cond>, Instruction *> > &front );
    virtual void         write_cpp_code_seq ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual bool         merge_predecessors ( Instruction **init = 0 );
    virtual std::string  code_repr          ();
    virtual void         get_code_repr      ( std::ostream &os ) = 0;
    virtual void         boyer_moore_opt    ( PtrPool<Instruction> &inst_pool, Instruction **init = 0 );
    virtual void         merge_eq_next      ( PtrPool<Instruction> &inst_pool );

    Context              cx;
    Vec<double>          freq;         ///< freq by char
    double               cum_freq;     ///< tot freq
    Vec<Transition>      next;         ///< transitions to other states
    Vec<Transition>      prev;         ///< transitions to other states

    // helpers for mark/rewind
    int                  num_in_dfs_stack;
    bool                 in_a_branch;
    bool                 in_a_cycle;
    Instruction         *orig;         ///< if is a clone (for a rewind)
    InstructionMark     *mark;         ///<

    // for code generation
    int                  num_ordering;
    unsigned             id_gen;

    // display and graph stuff
    std::string          _cache_code_repr;
    mutable unsigned     display_id;
    mutable unsigned     op_id;
    mutable unsigned     op_mp;
    static unsigned      cur_op_id;
};


} // namespace Hpipe

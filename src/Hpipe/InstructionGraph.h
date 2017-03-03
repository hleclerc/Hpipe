#pragma once

#include "Instruction.h"
#include "CharGraph.h"
#include "PtrPool.h"
#include "Context.h"

#include <unordered_map>
#include <queue>
#include <map>

namespace Hpipe {
class InstructionNextChar;
class InstructionRewind;
class InstructionMark;

/**
*/
class InstructionGraph {
public:
    InstructionGraph( CharGraph *cg, const std::vector<std::string> &disp = {}, bool disp_inst_pred = false, bool disp_trans_freq = false, bool want_boyer_moore = false );

    void                            apply               ( std::function<void(Instruction *)> f, bool subgraphs = false );
    int                             display_dot         ( CharGraph *cg = 0, bool disp_inst_pred = false, bool disp_trans_freq = false, bool disp_rc_item = false, const char *f = ".inst.dot", const char *prg = 0 ) const;
    std::string                     methods             () const { return cg->methods(); }
    Instruction                    *root                ();

    CharGraph                      *cg;
protected:
    struct PendingTrans {
        PendingTrans( Instruction *inst, unsigned num_edge, const Context &cx, const Vec<unsigned> &rcitem = {} ) : inst( inst ), num_edge( num_edge ), cx( cx ), rcitem( rcitem ), res( 0 ) {}
        Instruction  *inst;
        unsigned      num_edge;
        Context       cx;
        Vec<unsigned> rcitem;
        Instruction  *res;
    };
    struct PendingRewindTrans {
        PendingRewindTrans( Instruction *inst, unsigned num_trans, InstructionMark *rewind_mark, Instruction *res = 0, bool use_rv = false ) : inst( inst ), num_trans( num_trans ), rewind_mark( rewind_mark ), res( res ), use_rv( use_rv ) {}

        Instruction     *inst;
        unsigned         num_trans; ///< in inst
        InstructionMark *rewind_mark;
        Instruction     *res;
        bool             use_rv;    ///< use range_vec for nrcitem
    };
    struct RewindContext {
        bool operator<( const RewindContext &rw ) const { return std::tie( orig, mark ) < std::tie( rw.orig, rw.mark ); }
        Instruction *orig;
        Instruction *mark;
    };
    // using TCacheRewind = std::map<Context::PC,Instruction *>;

    void                            make_init            ();
    Instruction                    *make_transitions     ( Vec<PendingTrans> &pending_trans, const Context &cx, bool avoid_cycles, PendingTrans pt );
    // bool                            leads_to_ok        ( const Vec<const CharItem *> &items );
    // bool                            leads_to_ok_rec    ( const Vec<const CharItem *> &items, std::set<Vec<const CharItem *>> &visited );
    void                            train                ( bool only_cont = false );
    void                            remove_unused        ();
    void                            optimize_conditions  ();
    void                            make_marks_data       ( Instruction *root );
    void                            make_rewind_exec     ( InstructionMark *mark, InstructionRewind *rewind );
    void                            get_possible_inst_rec( std::set<std::pair<Instruction *,unsigned>> &possible_instructions, Instruction *inst, unsigned pos, const InstructionMark *mark );
    void                            disp_if              ( const std::vector<std::string> &disp, bool disp_inst_pred, bool disp_trans_freq, const std::string &name, bool disp_rcitem = true );
    void                            merge_eq_pred        ( Instruction *&root );
    void                            boyer_moore          ();
    Instruction                    *make_rewind_inst     ( Vec<PendingRewindTrans> &loc_pending_trans, std::map<RewindContext,Instruction *> &instruction_map, std::unordered_map<Instruction *,Vec<unsigned>> possible_inst, InstructionRewind *rewind, Instruction *orig, const PendingRewindTrans &pt ); ///< rewind_mark is a mark in the rewind context
    unsigned                        nb_multi_conds       ();
    Instruction                    *make_boyer_moore_rec ( const Vec<std::pair<Vec<Cond>, Instruction *> > &front, InstructionNextChar *next_char, int orig_front_size );
    bool                            can_make_a_rewind    ( std::set<std::pair<Instruction *,unsigned>> &possible_instructions, InstructionMark *mark, Instruction *inst, Vec<unsigned> rcitem );

    Instruction                    *init;
    Instruction                    *ok;
    Instruction                    *ko;
    Context                         cx_ok;
    std::map<Context,Instruction *> cache;
    std::set<Context>               forbiden_branching;
    // TCacheRewind                    cache_rewind;
    PtrPool<Instruction>            inst_pool;
};

} // namespace Hpipe

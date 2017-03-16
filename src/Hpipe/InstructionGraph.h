#pragma once

#include "Instruction.h"
#include "CharGraph.h"
#include "PtrPool.h"
#include "Context.h"

#include <unordered_map>
#include <deque>
#include <map>

namespace Hpipe {
class InstructionNextChar;
class InstructionRewind;
class InstructionMark;

/**
*/
class InstructionGraph {
public:
    InstructionGraph( CharGraph *cg, const std::vector<std::string> &disp = {}, int stop_char = -1, bool disp_inst_pred = false, bool disp_trans_freq = false, bool want_boyer_moore = false, bool no_training = false );

    void                            apply               ( std::function<void(Instruction *)> f, bool subgraphs = false );
    int                             display_dot         ( CharGraph *cg = 0, bool disp_inst_pred = false, bool disp_trans_freq = false, bool disp_rc_item = false, const char *f = ".inst.dot", const char *prg = 0 ) const;
    std::string                     methods             () const { return cg->methods(); }
    Instruction                    *root                ();

    CharGraph                      *cg;
protected:
    struct PendingTrans {
        PendingTrans( Instruction *inst, unsigned num_edge, const Context &cx, const Vec<unsigned> &rcitem = {} ) : inst( inst ), num_edge( num_edge ), cx( cx ), rcitem( rcitem ) {}
        PendingTrans( Instruction *inst, unsigned num_edge, const Context::PC &pc ) : inst( inst ), num_edge( num_edge ), cx( pc.first ), rcitem( pc.second ) {}

        Instruction  *inst;     ///< starting instruction
        unsigned      num_edge; ///< num transition in inst ( inst->next[ num_edge ] )
        Context       cx;       ///< where to go
        Vec<unsigned> rcitem;   ///< how to go to cx
    };

    struct PendingRewindTrans {
        PendingRewindTrans( Instruction *new_inst, unsigned num_prev, unsigned num_next ) : new_inst( new_inst ), num_prev( num_prev ), num_next( num_next ) {}

        Instruction *new_inst; ///<
        unsigned     num_prev; ///<
        unsigned     num_next; ///< num_next in prev
    };

    struct RewindContext {
        bool operator<( const RewindContext &rw ) const { return std::tie( orig, keep ) < std::tie( rw.orig, rw.keep ); }

        Instruction  *orig;
        Vec<unsigned> keep;
    };
    using Tcache = std::map<Context,Instruction *>;

    void                            make_init            ();
    Instruction                    *make_transitions     ( std::deque<PendingTrans> &pending_trans, const PendingTrans &pt );
    Instruction                    *make_transitions     ( std::deque<PendingTrans> &pending_trans, const PendingTrans &pt, const Context &cx );
    void                            simplify_marks       ();
    void                            train                ( bool only_cont = false );
    void                            remove_unused        ( Instruction *&root );
    void                            opt_stop_char        ( int stop_char );
    void                            optimize_conditions  ();
    void                            make_rewind_data     ( InstructionRewind *rw );
    void                            make_rewind_exec     ( InstructionMark *mark, InstructionRewind *rewind );
    void                            get_possible_inst_rec( std::set<std::pair<Instruction *,unsigned>> &possible_instructions, Instruction *inst, unsigned pos, const Instruction *mark );
    void                            disp_if              ( const std::vector<std::string> &disp, bool disp_inst_pred, bool disp_trans_freq, const std::string &name, bool disp_rcitem = true );
    void                            merge_eq_pred        ( Instruction *&root );
    void                            boyer_moore          ();
    unsigned                        nb_multi_conds       ();
    Instruction                    *make_boyer_moore_rec ( const Vec<std::pair<Vec<Cond>, Instruction *> > &front, InstructionNextChar *next_char, int orig_front_size );
    bool                            no_code_ambiguity    ( InstructionMark *mark, Instruction *inst, const Vec<unsigned> &rcitem );

    Instruction                    *ok;
    Instruction                    *ko;
    Instruction                    *init;
    Context                         cx_ok;
    Context                         cx_ko;
    Tcache                          cache;
    PtrPool<Instruction>            inst_pool;
    bool                            no_training;
};

} // namespace Hpipe

#include "InstructionTestContiguous.h"
#include "InstructionSkipBytes.h"
#include "InstructionMultiCond.h"
#include "InstructionNextChar.h"
#include "InstructionFreeStr.h"
#include "InstructionRewind.h"
#include "InstructionAddStr.h"
#include "InstructionClrStr.h"
#include "InstructionBegStr.h"
#include "InstructionEndStr.h"
#include "InstructionGraph.h"
#include "InstructionNone.h"
#include "InstructionMark.h"
#include "InstructionSkip.h"
#include "InstructionCode.h"
#include "InstructionCond.h"
#include "InstructionSave.h"
#include "InstructionEof.h"
#include "InstructionIf.h"
#include "InstructionOK.h"
#include "InstructionKO.h"
#include "DotOut.h"
#include "Assert.h"

#include <algorithm>
#include <queue>
#include <set>

namespace Hpipe {

InstructionGraph::InstructionGraph() {
    stop_char        = -1;
    no_training      = false;
    boyer_moore = false;
}

void InstructionGraph::read( CharGraph *cg, const std::vector<std::string> &disp, bool disp_inst_pred, bool disp_trans_freq ) {
    for( const std::string &str : cg->preliminaries ) preliminaries.push_back_unique( str );
    for( const std::string &str : cg->includes      ) includes     .push_back_unique( str );
    for( const std::string &str : cg->methods       ) methods      .push_back_unique( str );
    cx_ok = Context{ cg->char_item_ok };
    this->cg = cg;

    // predefined instructions
    ok = inst_pool << new InstructionOK( cx_ok );
    ko = inst_pool << new InstructionKO( cx_ko );
    cache[ cx_ok ] = ok;
    cache[ cx_ko ] = ko;

    // first graph
    make_init();
    disp_if( disp, disp_inst_pred, disp_trans_freq, "init" );

    // remove marks if not used
    simplify_marks();
    disp_if( disp, disp_inst_pred, disp_trans_freq, "mark" );

    // +1( MC, KO ) => +1[ no test ]( MC( 0 => KO, ... ) )
    if ( stop_char >= 0 )
        opt_stop_char( stop_char );
    disp_if( disp, disp_inst_pred, disp_trans_freq, "stop_char", false );

    // clean up
    remove_unused( init );
    disp_if( disp, disp_inst_pred, disp_trans_freq, "unused", false );

    // eq trans, eq pred (beware: rcitem becomes wrong)
    merge_eq_pred( init );
    disp_if( disp, disp_inst_pred, disp_trans_freq, "merge", false );

    // boyer-moore like optimizations
    if ( boyer_moore )
        make_boyer_moore();
    disp_if( disp, disp_inst_pred, disp_trans_freq, "boyer", false );

    // cond opt
    optimize_conditions();
    disp_if( disp, disp_inst_pred, disp_trans_freq, "cond", false );

    // merge (again) similar predecessors
    merge_eq_pred( init );
    disp_if( disp, disp_inst_pred, disp_trans_freq, "final", false );
}

void InstructionGraph::disp_if( const std::vector<std::string> &disp, bool disp_inst_pred, bool disp_trans_freq, const std::string &name, bool disp_rcitem ) {
    if ( std::find( disp.begin(), disp.end(), name ) != disp.end() ) {
        if ( disp_trans_freq )
            train( true );
        display_dot( cg, disp_inst_pred, disp_trans_freq, disp_rcitem, name.c_str() );
    }
}

int InstructionGraph::display_dot( CharGraph *cg, bool disp_inst_pred, bool disp_trans_freq, bool disp_rc_item, const char *f, const char *prg ) const {
    std::ofstream os( f );

    os << "digraph InstructionGraph {\n";

    if ( cg ) {
        ++CharItem::cur_op_id;
        cg->root()->write_dot_rec( os );
    }

    if ( init ) {
        std::map<InstructionMark *,Vec<Instruction *>> insts_by_mark;
        init->apply( [&]( Instruction *inst ) {
            insts_by_mark[ inst->mark ] << inst;
        } );

        ++Instruction::cur_op_id;
        // init->write_dot_rec( os, disp_inst_pred, disp_trans_freq, disp_rc_item );
        for( auto &p : insts_by_mark ) {
            //            os << "  subgraph cluster_" << p.first << " {\n";
            //            os << "  label = \"mark_" << p.first << "\";\n";
            //            os << "  color = red;\n";
            for( Instruction *inst : p.second )
                inst->write_dot_rec( os, disp_inst_pred, disp_trans_freq, disp_rc_item, false );
            //            os << "  }\n";
        }
    }

    os << "}\n";

    os.close();

    return exec_dot( f, prg );
}

Instruction *InstructionGraph::root() {
    return init;
}

void InstructionGraph::make_init() {
    // as much as possible, we try to move forward without cycles (to avoid miss of possible "free mark")
    std::deque<InstructionRewind *> pending_rewinds;
    std::deque<PendingTrans> pending_trans, pending_trans_mark;
    init = inst_pool << new InstructionSkip( Context( cg->root(), Context::FL_BEG ) );

    // first transition
    pending_trans.emplace_back( init, 0, init->cx.forward( cg->root() ) );

    // make all the remaining transitions
    while ( pending_trans.size() || pending_trans_mark.size() || pending_rewinds.size() ) {
        // if only rewinds => make rewind data (exec, ncx, ...) and add the new transitions
        if ( pending_trans.empty() && pending_trans_mark.empty() ) {
            while ( pending_rewinds.size() ) {
                InstructionRewind *rw = pending_rewinds.front();
                pending_rewinds.pop_front();
                make_rewind_data( rw );

                pending_trans.emplace_back( rw, 0, rw->ncx );
            }
            continue;
        }

        //
        std::deque<PendingTrans> &pt_list = pending_trans_mark.size() ? pending_trans_mark : pending_trans;
        PendingTrans &pt = pt_list.front();

        // make instruction with associated new pending transitions
        std::deque<PendingTrans> loc_pending_trans;
        Instruction *inst = make_transitions( loc_pending_trans, pt );
        for( PendingTrans &pt : loc_pending_trans )
            ( pt.cx.mark ? pending_trans_mark : pending_trans ).push_back( std::move( pt ) );
        if ( InstructionRewind *rw = dynamic_cast<InstructionRewind *>( inst ) )
            pending_rewinds.push_back( rw );

        // insert it into the graph
        pt.inst->next.secure_set( pt.num_edge, { inst, pt.rcitem } );
        inst->prev.emplace_back( pt.inst, pt.rcitem );

        //
        pt_list.pop_front();
    }
}

Instruction *InstructionGraph::make_transitions( std::deque<PendingTrans> &pending_trans, const PendingTrans &pt ) {
    return make_transitions( pending_trans, pt, pt.cx );
}

Instruction *InstructionGraph::make_transitions( std::deque<PendingTrans> &pending_trans, const PendingTrans &pt, const Context &cx ) {
    // normalization
    if ( ( cx.flags & Context::FL_OK ) && CharGraph::leads_to_ok( cx.pos ) )
        return make_transitions( pending_trans, pt, cx.without_flag( Context::FL_OK ) );

    // helper func
    auto tra = [&]( Instruction *inst, unsigned num_edge, const Context::PC &pc ) {
        pending_trans.emplace_back( inst, num_edge, pc.first, pc.second );
        return inst;
    };

    // this transition is going to cancel a BEG_STR ?
    if ( ! cx.mark ) {
        Vec<std::string> strs = pt.inst->strs_not_in( cx );
        if ( ! strs.empty() )
            return tra( new InstructionFreeStr( cx, strs ), 0, { cx, range_vec( unsigned( cx.pos.size() ) ) } );
    }

    // we can cancel a mark ?
    if ( cx.mark && ( CharGraph::leads_to_ok( cx.pos ) || cx.pos.empty() ) ) {
        if ( cx.paths_to_mark.contains( cx.mark->num_active_item ) == false ||
             cx.paths_to_mark.only_has( cx.mark->num_active_item ) ) {
            InstructionRewind *res = inst_pool << new InstructionRewind( cx );
            // res->running_strs = pt.inst->running_strs;
            return res;
        }
    }

    // we already have way(s) to go to cx ?
    Tcache::iterator iter = cache.find( cx );
    if ( iter != cache.end() )
        return iter->second;

    // helper func
    auto reg = [&]( auto *inst ) {
        cache.insert( iter, std::make_pair( cx, inst_pool << inst ) );
        return inst;
    };

    // dead end ?
    if ( cx.pos.empty() )
        return ko;

    // something to skip ?
    for( const CharItem *item : cx.pos )
        if ( item->type == CharItem::BEGIN or item->type == CharItem::PIVOT or item->type == CharItem::LABEL )
            return tra( reg( new InstructionSkip( cx ) ), 0, cx.forward( item ) );

    // everything is OK ?
    if ( cx.only_has( CharItem::OK ) )
        return ok;

    // lonely skip bytes
    if ( cx.pos[ 0 ]->type == CharItem::SKIP && ( cx.pos.size() == 1 || cx.pos[ 2 ]->type == CharItem::OK ) ) {
        InstructionSkipBytes *res = reg( new InstructionSkipBytes( cx, cx.pos[ 0 ]->str, cx.beg() ) );
        tra( res, 0, cx.forward( cx.pos[ 0 ] ) );
        tra( res, 1, cx.without( cx.pos[ 0 ] ) );
        return res;
    }

    // several skips
    if ( cx.has( CharItem::SKIP ) ) {
        cg->err( "TODO: skip instructions with parallel paths" );
        HPIPE_TODO;
    }

    // EOF test ?
    for( const CharItem *item : cx.pos ) {
        if ( item->type == CharItem::_EOF ) {
            InstructionEof *res = reg( new InstructionEof( cx, cx.beg() ) );
            tra( res, 0, cx.with_flag( Context::FL_NOT_EOF ).without( item ) );
            tra( res, 1, cx.with_flag( Context::FL_EOF  ).forward( item ) );
            return res;
        }
    }

    // BEG_STR (we're not going to add a mark for a BEG_STR, even if there is an ambiguity)
    if ( ! cx.mark ) {
        for( unsigned ind = 0; ind < cx.pos.size(); ++ind ) {
            const CharItem *item = cx.pos[ ind ];
            if ( ( item->type == CharItem::BEG_STR || item->type == CharItem::BEG_STR_NEXT ) && cx.paths_to_strings.count( item->str ) == 0 )
                return tra( new InstructionBegStr( cx, item->str, ind, item->type == CharItem::BEG_STR_NEXT ), 0, cx.with_string( item->str ).forward( item ) );
        }
    }

    // we have a code ?
    for( unsigned ind = 0; ind < cx.pos.size(); ++ind ) {
        const CharItem *item = cx.pos[ ind ];
        if ( item->code_like() ) {
            // if it's too early to decide if this code can be executed, we have to add a mark
            if ( not cx.mark and ( cx.pos.size() >= 2 or not cx.leads_to_ok() ) ) {
                InstructionMark *res = reg( new InstructionMark( cx, ind ) );
                return tra( res, 0, cx.with_mark( res ) );
            }

            // Instruction type
            Instruction *res = 0;
            switch( item->type ) {
            case CharItem::CODE        : res = reg( new InstructionCode  ( cx, item->str, ind ) ); break;
            case CharItem::ADD_STR     : res = reg( new InstructionAddStr( cx, item->str, ind ) ); break;
            case CharItem::CLR_STR     : res = reg( new InstructionClrStr( cx, item->str, ind ) ); break;
            case CharItem::BEG_STR     : res = reg( new InstructionBegStr( cx, item->str, ind, 0 ) ); break;
            case CharItem::BEG_STR_NEXT: res = reg( new InstructionBegStr( cx, item->str, ind, 1 ) ); break;
            case CharItem::END_STR     : res = reg( new InstructionEndStr( cx, item->str, ind, 0 ) ); if ( cx.mark ) break; return tra( res, 0, cx.without_string( item->str ).forward( item ) );
            case CharItem::END_STR_NEXT: res = reg( new InstructionEndStr( cx, item->str, ind, 1 ) ); if ( cx.mark ) break; return tra( res, 0, cx.without_string( item->str ).forward( item ) );
            default: HPIPE_TODO;
            }

            //
            //            if ( not cx.mark and cx.beg() and res->data_code() )
            //                cg->err( "A code cannot correctly depend on a char data before a first char has been queried" );

            // transition
            return tra( res, 0, cx.forward( item ) );
        }
    }

    // now we should have only conds, ifs, OKs and +1s. -> do the conds
    for( const CharItem *i : cx.pos ) {
        if ( i->type == CharItem::COND ) {
            if ( cx.beg() )
                cg->err( "A condition cannot appear before a first char has been queried" );
            Cond covered;
            Vec<Cond> conds;
            for( const CharItem *item : cx.pos ) {
                HPIPE_ASSERT( item->type == CharItem::COND or item->type == CharItem::NEXT_CHAR or item->type == CharItem::OK, "" );
                if ( item->type != CharItem::COND )
                    continue;
                covered |= item->cond;

                Vec<Cond> old_conds = conds;
                Cond t = item->cond;
                conds.clear();
                for( const Cond &c : old_conds ) {
                    if ( Cond i = t & c )
                        conds << i;
                    if ( Cond i = ~t & c )
                        conds << i;
                    t &= ~c;
                }
                if ( t )
                    conds << t;
            }


            //
            if ( conds.size() == 1 and covered.always_checked() )
                return tra( reg( new InstructionSkip( cx ) ), 0, cx.forward( CharItem::COND ) );

            // make transition and instruction for each cond in `conds`
            InstructionMultiCond *res = reg( new InstructionMultiCond( cx, conds ) );
            unsigned cpt = 0;
            for( const Cond &c : conds )
                tra( res, cpt++, cx.forward( c ) );
            if ( not covered.always_checked() ) {
                tra( res, cpt++, cx.forward( ~covered ) );
                res->conds << ~covered;
            }
            return res;
        }
    }

    // ifs ?
    for( unsigned ind = 0; ind < cx.pos.size(); ++ind ) {
        const CharItem *item = cx.pos[ ind ];
        if ( item->type == CharItem::_IF ) {
            InstructionIf *res = reg( new InstructionIf( cx, item->str, ind ) );
            tra( res, 0, cx.forward( item ) );
            tra( res, 1, cx.without( item ) );
            return res;
        }
    }

    // at this point, we should have only OKs and +1s
    for( const CharItem *item : cx.pos )
        HPIPE_ASSERT( item->type == CharItem::OK or item->type == CharItem::NEXT_CHAR, "" );

    int first_leads_to_ok = -1;
    if ( cx.only_has( CharItem::NEXT_CHAR ) ) {
        for( unsigned i = 0; i < cx.pos.size(); ++i ) {
            if ( CharGraph::leads_to_ok( cx.pos[ i ] ) ) {
                first_leads_to_ok = i;
                break;
            }
        }
    }

    if ( cx.eof() ) {
        if ( first_leads_to_ok >= 0 ) { // result will be an OK instruction, with an edge enabling a rewind
            Context ncx_ok( cg->char_item_ok );
            return tra( reg( new InstructionSkip( cx ) ), 0, Context::PC{ ncx_ok, first_leads_to_ok } );
        }
        if ( cx.has( CharItem::OK ) )
            return tra( new InstructionSkip( cx ), 0, cx.only_with( CharItem::OK ) );
        return tra( reg( new InstructionSkip( cx ) ), 0, Context::PC{ {}, {} } );
    }

    Instruction *res = reg( new InstructionNextChar( cx, cx.beg() ) );
    tra( res, 0, cx.without_flag( Context::FL_BEG ).without_flag( Context::FL_NOT_EOF ).forward( CharItem::NEXT_CHAR ) ); // if OK

    if ( not cx.not_eof() ) {
        if ( first_leads_to_ok >= 0 ) { // result will be an OK instruction, with an edge enabling a rewind
            Context ncx_ok( cg->char_item_ok, cx.flags | Context::FL_EOF );
            tra( res, 1, Context::PC{ ncx_ok, first_leads_to_ok } ); // if no next char
        } else {
            tra( res, 1, cx.without_flag( Context::FL_EOF ).only_with( CharItem::OK ) ); // if no next char
        }
    }
    return res;
}

void remove_mark_rec( Instruction *inst, InstructionMark *mark ) {
    if ( inst->mark != mark )
        return;
    inst->mark = 0;

    if ( InstructionWithCode *wc = dynamic_cast<InstructionWithCode *>( inst ) )
        wc->remove();

    for( Transition &p : inst->next )
        remove_mark_rec( p.inst, mark );
}

void InstructionGraph::simplify_marks() {
    // get all the marks
    Vec<InstructionMark *> marks;
    root()->apply( [&]( Instruction *inst ) {
        if ( InstructionMark *mc = dynamic_cast<InstructionMark *>( inst ) )
            marks << mc;
    } );

    // find marks that can be removed
    for( InstructionMark *mark : marks ) {
        // move E0 and E-1 instructions after the rewinds
        for( InstructionRewind *rw : mark->rewinds ) {
            if ( rw->need_rw )
                continue;
            for( unsigned i = 0; i < rw->code_seq_end.size(); ++i ) {
                InstructionRewind::CodeSeqItem &item = rw->code_seq_end[ i ];
                if ( item.offset > 0 )
                    break;
                Instruction *ninst = item.code->clone( inst_pool, item.code->cx, range_vec( unsigned( item.code->cx.pos.size() ) ) );
                rw->code_seq_end.erase( rw->code_seq_end.begin() + i-- );
                rw->insert_after_this( ninst );
            }
        }

        // need_mark
        bool need_mark = false;
        for( InstructionRewind *rw : mark->rewinds ) {
            if ( rw->need_mark() ) {
                need_mark = true;
                PRINT( *rw );
                break;
            }
        }

        if ( ! need_mark ) {
            for( InstructionRewind *rw : mark->rewinds ) {
                if ( rw->no_code_at_all() )
                    rw->remove();
                else
                    rw->mark = 0;
            }
            for( Transition &p : mark->next )
                remove_mark_rec( p.inst, mark );
            mark->remove();
        }
    }
}

void InstructionGraph::train( bool only_cont ) {
    Vec<Lexer::TrainingData> tds = cg->training_data();
    if ( tds.size() ) {
        init->apply( []( Instruction *inst ) {
            for( Transition &t : inst->next )
                t.freq = 0.0;
            inst->freq.assign( 256, 0.0 );
            inst->cum_freq = 0.0;
        }, true );

        for( const Lexer::TrainingData &td : tds ) {
            const std::string inp = td.inp;
            // without contiguous data
            std::string::size_type s = 0, m;
            if ( not only_cont ) {
                init->train_rec( s, m, inp, td.freq, false );
                // with contiguous data
                s = 0;
            }
            init->train_rec( s, m, inp, td.freq, true );
        }
    }
}

void InstructionGraph::remove_unused( Instruction *&root ) {
    // list of instructions that can be removed from the graph
    Vec<Instruction *> to_remove;
    ++Instruction::cur_op_id;
    if ( root )
        root->get_unused_rec( to_remove, root );

    // remove them
    for( Instruction *inst : to_remove )
        inst->remove( false );
}

void InstructionGraph::opt_stop_char( int stop_char ) {
    // get all MultiCond
    Vec<InstructionMultiCond *> mcs;
    root()->apply( [&]( Instruction *inst ) {
        if ( InstructionMultiCond *mc = dynamic_cast<InstructionMultiCond *>( inst ) )
            mcs << mc;;
    } );

    // each multicond with prev
    for( InstructionMultiCond *mc : mcs ) {
        // we want all prev to be a NextChar, prev->ok to be mc, and all prev->ko to be the same
        if ( mc->prev.empty() )
            continue;
        InstructionNextChar *fc = dynamic_cast<InstructionNextChar *>( mc->prev[ 0 ].inst );
        if ( ! fc )
            continue;
        bool same_ko = true;
        Instruction *ko = fc->next.size() >= 2 ? fc->next[ 1 ].inst : 0;
        for( int i = 0; i < mc->prev.size(); ++i ) {
            InstructionNextChar *nc = dynamic_cast<InstructionNextChar *>( mc->prev[ i ].inst );
            if ( nc == 0 || nc->next[ 0 ].inst != mc || ( ko ? nc->next[ 1 ].inst != ko : nc->next.size() < 2 ) ) {
                same_ko = false;
                break;
            }
        }
        if ( ! same_ko )
            continue;

        // check that ko does not contain positionnal code or if its possible to offset the positionnal code
        if ( ko ) {
            bool ok_ko = true, has_choice = false;
            Vec<InstructionRewind *> rewinds;
            ko->apply( [&]( Instruction *inst ) {
                // we want rewinds to be updated only once
                if ( inst->prev.size() > 1 )
                    has_choice = true;
                if ( InstructionRewind *rw = dynamic_cast<InstructionRewind *>( inst ) ) {
                    if ( rw->need_rw || has_choice )
                        ok_ko = false;
                    else
                        rewinds << rw;
                } else if ( inst->data_code() || dynamic_cast<InstructionMark *>( inst ) )
                    ok_ko = false;
            } );
            if ( ! ok_ko )
                continue;
            // update offset in rewinds
            if ( ! fc->beg )
                for( InstructionRewind *rw : rewinds )
                    for( InstructionRewind::CodeSeqItem &item : rw->code_seq_end )
                        if ( item.offset >= 0 )
                            ++item.offset;
        }

        // remove nc->ko
        for( int i = 0; i < mc->prev.size(); ++i ) {
            InstructionNextChar *nc = static_cast<InstructionNextChar *>( mc->prev[ i ].inst );
            ko->prev.remove_first_checking( [&]( Transition &prev ) { return prev.inst == nc; } );
            nc->next.resize( 1 );
        }

        // add cond to mc
        Cond zc( stop_char );
        for( Cond &c : mc->conds )
            c &= ~zc;
        mc->conds.emplace_back( zc );
        mc->next.emplace_back( ko );
        ko->prev.emplace_back( mc );
    }
}

void InstructionGraph::optimize_conditions() {
    if ( ! no_training )
        train();
    
    // unsigned nb_conds = 0;
    // root()->apply( [&]( Instruction *inst ) {
    //     nb_conds += dynamic_cast<InstructionMultiCond *>( inst ) != 0;
    // } );

    ++Instruction::cur_op_id;
    init->optimize_conditions( inst_pool );
}

void InstructionGraph::merge_eq_pred( Instruction *&root ) {
    if ( ! root )
        return;

    // set inst->_cache_code_repr to avoid ++Instruction::cur_op_id inside the find_rec
    root->apply( []( Instruction *inst ) {
        inst->code_repr();
    } );

    // while possible to merge something
    while ( true ) {
        ++Instruction::cur_op_id;
        if ( not root->find_rec( [ &root ]( Instruction *inst ) { return inst->merge_predecessors( &root ); } ) )
            break;
    }

    root->apply( [&]( Instruction *inst ) {
        inst->merge_eq_next( inst_pool );
    } );
}

void InstructionGraph::make_boyer_moore() {
    // get NextChar items
    //++Instruction::cur_op_id;
    //init->boyer_moore_opt( inst_pool, &init );

    // the cost can become very high... so we limit the number of added conditions
    unsigned max_conds = std::max( 10 * nb_multi_conds(), 1000u );
    unsigned max_jump_size = 10;
    std::set<InstructionNextChar *> boyer_moored;
    for( unsigned cpt = 0; ; ++cpt ) {
        // look up for the most used InstructionNextChar
        train( true );
        bool has_code = false;
        InstructionNextChar *next_char = 0;
        root()->apply( [&]( Instruction *inst ) {
            if ( InstructionNextChar *nc = dynamic_cast<InstructionNextChar *>( inst ) )
                if ( ( next_char == 0 or nc->cum_freq > next_char->cum_freq ) and not boyer_moored.count( nc ) ) 
                    next_char = nc;
            if ( not has_code )
                has_code = inst->with_code();
        } );
        if ( not next_char or not has_code )
            break;
        boyer_moored.insert( next_char );

        // make possible path (upto something that produces code, or max_jump_size limit)
        Vec<std::pair<Vec<Cond>,Instruction *>> front; // seq of conds to instructions with code
        front.emplace_back( Cond{ 0, 255 }, next_char->next[ 0 ].inst );
        while ( true ) {
            // use conds, keep +1 and skip the other kinds of instructions
            bool has_code = false;
            for( unsigned num_in_front = 0; num_in_front < front.size(); ++num_in_front ) {
                Instruction *&inst = front[ num_in_front ].second;

                // code
                if ( inst->with_code() or
                     dynamic_cast<InstructionRewind *>( inst ) or
                     dynamic_cast<InstructionSave   *>( inst ) or
                     dynamic_cast<InstructionMark   *>( inst ) or
                     dynamic_cast<InstructionOK     *>( inst ) or
                     dynamic_cast<InstructionKO     *>( inst )
                     ) {
                    has_code = true;
                    continue;
                }

                // +1 (handled in the next loop)
                if ( dynamic_cast<InstructionNextChar *>( inst ) )
                    continue;

                // conditions
                if ( InstructionMultiCond *cond = dynamic_cast<InstructionMultiCond *>( inst ) ) {
                    Vec<Cond> cond_seq_b = std::move( front[ num_in_front ].first );
                    front.remove_unordered( num_in_front-- );

                    for( unsigned i = 0; i < cond->next.size(); ++i) {
                        Vec<Cond> cond_seq = cond_seq_b;
                        cond_seq.back() &= cond->conds[ i ];
                        front.emplace_back( std::move( cond_seq ), cond->next[ i ].inst );
                    }
                    continue;
                }

                // skip the other kinds of instructions
                if ( dynamic_cast<InstructionTestContiguous *>( inst ) )
                    inst = inst->next[ 1 ].inst;
                else
                    inst = inst->next[ 0 ].inst;
                --num_in_front;
            }

            if ( has_code )
                break;
            if ( front[ 0 ].first.size() >= max_jump_size ) {
                std::cerr << "Information: boyer-moore optimization has been stopped due to max_jump_size." << std::endl;
                break;
            }

            // use next_chars
            for( unsigned num_in_front = 0; num_in_front < front.size(); ++num_in_front ) {
                Instruction *&inst = front[ num_in_front ].second;
                inst = inst->next[ 0 ].inst;
                front[ num_in_front ].first.emplace_back( 0, 255 );
            }
        }

        //  if only 1 cond -> nothing to optimize
        if ( front.size() == 0 or front[ 0 ].first.size() <= 1 )
            continue;

        //
        InstructionTestContiguous *test = inst_pool << new InstructionTestContiguous( next_char->cx, next_char->beg, front[ 0 ].first.size() );
        Instruction *ok = make_boyer_moore_rec( front, next_char, front[ 0 ].first.size() );
        test->next << ok;
        ok->prev << test;

        next_char->insert_before_this( test, init );

        if ( nb_multi_conds() >= max_conds ) {
            std::cerr << "Information: boyer-moore optimization has been stopped after " << cpt << " iterations." << std::endl;
            break;
        }
    }


    //    // get possibilities
    //    std::map<unsigned,unsigned> sizes;
    //    for( InstructionNextChar *next_char : next_chars ) {
    //        Vec<std::pair<Vec<Cond>,Instruction *>> front; // seq of conds to instructions with code
    //        front.emplace_back( Cond{ 0, 255 }, next_char->next[ 0 ].inst );
    //        while ( true ) {
    //            // use conds, keep +1 and skip the other kinds of instructions
    //            bool has_code = false;
    //            for( unsigned num_in_front = 0; num_in_front < front.size(); ++num_in_front ) {
    //                Instruction *&inst = front[ num_in_front ].second;

    //                // code
    //                if ( inst->with_code() or
    //                     dynamic_cast<InstructionRewind *>( inst ) or
    //                     dynamic_cast<InstructionSave   *>( inst ) or
    //                     dynamic_cast<InstructionMark   *>( inst ) or
    //                     dynamic_cast<InstructionOK     *>( inst ) or
    //                     dynamic_cast<InstructionKO     *>( inst )
    //                   ) {
    //                    has_code = true;
    //                    continue;
    //                }

    //                // +1 (handled in the next loop)
    //                if ( dynamic_cast<InstructionNextChar *>( inst ) )
    //                    continue;

    //                // conditions
    //                if ( InstructionMultiCond *cond = dynamic_cast<InstructionMultiCond *>( inst ) ) {
    //                    Vec<Cond> cond_seq_b = std::move( front[ num_in_front ].first );
    //                    front.remove_unordered( num_in_front-- );

    //                    for( unsigned i = 0; i < cond->next.size(); ++i) {
    //                        Vec<Cond> cond_seq = cond_seq_b;
    //                        cond_seq.back() &= cond->conds[ i ];
    //                        front.emplace_back( std::move( cond_seq ), cond->next[ i ].inst );
    //                    }
    //                    continue;
    //                }

    //                // skip the other kinds of instructions
    //                if ( dynamic_cast<InstructionTestContiguous *>( inst ) )
    //                    inst = inst->next[ 1 ].inst;
    //                else
    //                    inst = inst->next[ 0 ].inst;
    //                --num_in_front;
    //            }

    //            if ( has_code )
    //                break;

    //            // use next_chars
    //            for( unsigned num_in_front = 0; num_in_front < front.size(); ++num_in_front ) {
    //                Instruction *&inst = front[ num_in_front ].second;
    //                inst = inst->next[ 0 ].inst;
    //                front[ num_in_front ].first.emplace_back( 0, 255 );
    //            }
    //        }

    //        //  if only 1 cond -> nothing to optimize
    //        if ( front.size() == 0 or front[ 0 ].first.size() <= 1 )
    //            continue;
    //        ++sizes[ front[ 0 ].first.size() ];

    //        //
    //        InstructionTestContiguous *test = inst_pool << new InstructionTestContiguous( next_char->cx, next_char->beg, front[ 0 ].first.size() );
    //        Instruction *ok = make_boyer_moore_rec( front, next_char, front[ 0 ].first.size() );
    //        test->next << ok;
    //        ok->prev << test;


    //        next_char->insert_before_this( test, init );
    //    }
    
    //    std::cout << "Boyer-Moore sizes:";
    //    for( auto p : sizes )
    //        std::cout << " " << p.first << "(n=" << p.second << ")";
    //    std::cout << std::endl;
}

unsigned InstructionGraph::nb_multi_conds() {
    unsigned res = 0;
    root()->apply( [&]( Instruction *inst ) {
        res += dynamic_cast<InstructionMultiCond *>( inst ) != 0;
    } );
    return res;
}

Instruction *InstructionGraph::make_boyer_moore_rec( const Vec<std::pair<Vec<Cond>, Instruction *> > &front, InstructionNextChar *next_char, int orig_front_size ) {
    //
    auto same_inst = [&]() {
        Instruction *f = front[ 0 ].second;
        for( unsigned i = 1; i < front.size(); ++i )
            if ( front[ i ].second != f )
                return false;
        return true;
    };
    if ( same_inst() )
        return front[ 0 ].second;

    // get the conds to test
    Vec<Cond> conds;
    for( const auto &p : front ) {
        Vec<Cond> old_conds = conds;
        Cond t = p.first.back();
        conds.clear();
        for( const Cond &c : old_conds ) {
            if ( Cond i = t & c )
                conds << i;
            if ( Cond i = ~t & c )
                conds << i;
            t &= ~c;
        }
        if ( t )
            conds << t;
    }
    std::sort( conds.begin(), conds.end(), []( const Cond &a, const Cond &b ) {
        return a.nz() < b.nz();
    } );

    //
    InstructionMultiCond *res = inst_pool << new InstructionMultiCond( next_char->cx, conds, front[ 0 ].first.size() - orig_front_size );
    for( const Cond &cond : conds ) {
        Vec<std::pair<Vec<Cond>, Instruction *> > new_front;
        for( const std::pair<Vec<Cond>, Instruction *> &p : front )
            if ( p.first.back() & cond )
                new_front.emplace_back( p.first.up_to( p.first.size() - 1 ), p.second );

        Instruction *next = make_boyer_moore_rec( new_front, next_char, orig_front_size );
        res->next << next;
        next->prev << res;
    }

    return res;

}

bool InstructionGraph::no_code_ambiguity( InstructionMark *mark, Instruction *inst, const Vec<unsigned> &rcitem ) {
    if ( rcitem.empty() )
        return true;

    std::set<std::pair<Instruction *,unsigned>> possible_instructions;
    for( unsigned ind : rcitem )
        get_possible_inst_rec( possible_instructions, inst, ind, mark );

    //
    std::map<InstructionWithCode *,bool> possible_with_code; // true => active
    for( auto &p : possible_instructions ) {
        if ( InstructionWithCode *code = dynamic_cast<InstructionWithCode *>( p.first ) ) {
            std::map<InstructionWithCode *,bool>::iterator iter = possible_with_code.find( code );
            bool active = p.second == code->num_active_item;
            if ( iter == possible_with_code.end() )
                possible_with_code.insert( iter, std::make_pair( code, active ) );
            else if ( iter->second != active )
                return false;
        }
    }
    return true;
}

void InstructionGraph::get_possible_inst_rec( std::set<std::pair<Instruction *,unsigned>> &possible_instructions, Instruction *inst, unsigned pos, const Instruction *mark ) {
    auto iter = possible_instructions.find( { inst, pos } );
    if ( iter != possible_instructions.end() )
        return;
    possible_instructions.insert( iter, { inst, pos } );

    if ( inst != mark )
        for( const Transition &p : inst->prev )
            get_possible_inst_rec( possible_instructions, p.inst, p.rcitem[ pos ], mark );
}

void InstructionGraph::make_rewind_data( InstructionRewind *rewind ) {
    make_rewind_exec( rewind->mark, rewind );

    if ( ! rewind->exec ) {
        rewind->ncx = rewind->cx.without_mark();
        return;
    }

    // read instructions from the end (from OK instruction) until undecidabitity
    InstructionOK *ok = 0;
    unsigned offset_end = 0;
    rewind->exec->apply_rw( [&]( Instruction *inst ) {
        if ( InstructionOK *tr = dynamic_cast<InstructionOK *>( inst ) )
            ok = tr;
    } );
    if ( ok ) {
        for( Instruction *inst = ok, *next = nullptr; ; ) {
            // if in a loop, stop here
            if ( inst->nb_next_rw() >= 1 + bool( inst != ok ) )
                break;

            // some code to register ?
            if ( InstructionWithCode *wc = dynamic_cast<InstructionWithCode *>( inst ) ) {
                // if ambiguity, on a code, we stop here, to let the beg seq take care of it (with a rewind)
                if ( wc->cx.pos.size() >= 2 )
                    break;
                rewind->code_seq_end.emplace_back( wc->data_code() ? offset_end : -1, wc );
                wc->in_code_seq = true;
            }

            // several choices or end ?
            if ( inst->prev.size() != 1 )
                break;

            // advance
            if ( InstructionNextChar *nc = dynamic_cast<InstructionNextChar *>( inst ) ) {
                if ( nc->beg == false ) {
                    bool in[ 2 ] = { false, false }; // in ok, in ko
                    for( unsigned nt = 0; nt < 2; ++nt ) {
                        if ( nt < nc->next_rw.size() ) {
                            for( const Transition &t : nc->next_rw[ nt ] )
                                if ( next && t.inst == next )
                                    in[ nt ] = true;
                        }
                    }
                    // if OK and KO are possible, we don't know if we have to advance or not
                    if ( in[ 0 ] && in[ 1 ] )
                        break;
                    if ( in[ 0 ] )
                        ++offset_end;
                }
            }

            // loop
            next = inst;
            inst = inst->prev[ 0 ].inst;
        }
    }

    // replace _next instruction to non next versions if possible
    for( unsigned num_item = 0; num_item < rewind->code_seq_end.size(); ++num_item ) {
        InstructionRewind::CodeSeqItem &item = rewind->code_seq_end[ num_item ];
        if ( item.offset && item.code->works_on_next() && ( num_item == 0 || rewind->code_seq_end[ num_item - 1 ].offset < item.offset ) ) {
            item.code = item.code->no_works_on_next_clone( inst_pool );
            --item.offset;
        }
    }

    // read instructions from the beginning, until undecidability
    unsigned offset_beg = 0;
    const Context *ncx_beg = 0;
    for( Instruction *inst = rewind->exec; ; ) {
        ncx_beg = &inst->cx;

        // if in a loop, stop here
        if ( inst->prev.size() >= 1 + bool( inst != rewind->exec ) )
            break;

        // several choices / undecidability or no next
        Instruction *next_rw;
        unsigned nb_next_rw = 0;
        for( const Vec<Transition> &n : inst->next_rw ) {
            for( const Transition &t : n ) {
                if ( t.inst ) {
                    next_rw = t.inst;
                    ++nb_next_rw;
                }
            }
        }
        if ( nb_next_rw != 1 )
            break;

        // some code to register ?
        if ( InstructionWithCode *wc = dynamic_cast<InstructionWithCode *>( inst ) ) {
            // already handled by code_seq_end
            if ( wc->in_code_seq )
                break;
            // if ambiguity, we make a rewind on wc
            if ( wc->cx.pos.size() >= 2 ) {
                rewind->offset_ncx = offset_beg;
                rewind->need_rw = true;
                break;
            }
            // else, register in code_seq_beg
            rewind->code_seq_beg.emplace_back( wc->data_code() ? offset_beg : -1, wc );
            wc->in_code_seq = true;
        }

        // advance
        if ( InstructionNextChar *nc = dynamic_cast<InstructionNextChar *>( inst ) ) {
            if ( nc->beg == false && nc->next_rw[ 0 ].size() )
                ++offset_beg;
        }

        //
        inst = next_rw;
    }

    // test if there are some instructions between code_seq_beg and code_seq_end
    rewind->exec->apply_rw( [&]( Instruction *inst ) {
        if ( InstructionWithCode *code = dynamic_cast<InstructionWithCode *>( inst ) ) {
            if ( ! code->in_code_seq ) {
                rewind->offset_ncx = offset_beg;
                rewind->need_rw = true;
            }
        }
    } );

    // restart context
    if ( rewind->need_rw )
        rewind->ncx = ncx_beg->without_mark();
    else
        rewind->ncx = rewind->cx.without_mark();

    // stuff that may change the restart context
    for( const InstructionRewind::CodeSeqItem &csi : rewind->code_seq_beg ) {
        csi.code->update_running_strings( rewind->running_strs );

        if ( InstructionBegStr *bs = dynamic_cast<InstructionBegStr *>( csi.code ) )
            rewind->ncx.first.add_string( bs->var, bs->num_active_item );
        else if ( InstructionEndStr *es = dynamic_cast<InstructionEndStr *>( csi.code ) )
            rewind->ncx.first.rem_string( es->var );
    }
    if ( ! rewind->need_rw ) {
        for( unsigned i = rewind->code_seq_end.size(); i--; ) {
            const InstructionRewind::CodeSeqItem &csi = rewind->code_seq_end[ i ];
            csi.code->update_running_strings( rewind->running_strs );

            if ( InstructionBegStr *bs = dynamic_cast<InstructionBegStr *>( csi.code ) )
                rewind->ncx.first.add_string( bs->var, bs->num_active_item );
            else if ( InstructionEndStr *es = dynamic_cast<InstructionEndStr *>( csi.code ) )
                rewind->ncx.first.rem_string( es->var );
        }

    }

    //    // add save point, remove mark if possible
    //    for( InstructionMark *mark : marks ) {
    //        // make rewind->exec, update flags for mark
    //        bool remove_mark = true;
    //        unsigned num_save = 0;
    //        for( InstructionRewind *rewind : mark->rewinds ) {
    //            if ( rewind->need_rw )
    //                remove_mark = false;
    //            else if ( rewind->code_seq.size() ) {
    //                // add save points
    //                for( InstructionWithCode *code : rewind->code_seq ) {
    //                    if ( not code->data_code() )
    //                        continue;
    //                    Instruction *orig = code->orig;
    //                    while ( orig->prev.size() == 1 and dynamic_cast<InstructionWithCode *>( orig->prev[ 0 ].inst ) )
    //                        orig = orig->prev[ 0 ].inst;
    //                    if ( orig->prev.size() == 1 and dynamic_cast<InstructionSave *>( orig->prev[ 0 ].inst ) ) {
    //                        code->save = static_cast<InstructionSave *>( orig->prev[ 0 ].inst );
    //                        continue;
    //                    }
    //                    code->save = inst_pool << new InstructionSave( orig->cx, num_save++ );
    //                    orig->insert_before_this( code->save, init );
    //                    // code->save->mark = mark;
    //                }
    //            }
    //        }
}

void InstructionGraph::make_rewind_exec( InstructionMark *mark, InstructionRewind *rewind ) {
    // we start from the end
    std::map<RewindContext,Instruction *> instruction_map; // [ orig, keep_ind ] => new instruction
    std::deque<PendingRewindTrans> pending_trans; //

    auto reg = [&]( Instruction *orig, Vec<unsigned> orig_keep_ind ) {
        Instruction *new_inst = orig->clone( inst_pool, orig->cx.keep_only( orig_keep_ind ).first.without_mark().first, orig_keep_ind );
        instruction_map[ RewindContext{ orig, orig_keep_ind } ] = new_inst;
        new_inst->running_strs = orig->running_strs;
        new_inst->orig_keed_ind = orig_keep_ind;
        new_inst->orig = orig;

        // look into prevs
        if ( orig != mark ) {
            std::set<Instruction *> prevs;
            for( unsigned num_prev = 0; num_prev < orig->prev.size(); ++num_prev ) {
                Instruction *prev = orig->prev[ num_prev ].inst;
                if ( prevs.count( prev ) )
                    continue;
                prevs.insert( prev );
                for( unsigned num_next = 0; num_next < prev->next.size(); ++num_next )
                    if ( prev->next[ num_next ].inst == orig )
                        pending_trans.emplace_back( new_inst, num_prev, num_next );
            }
        }

        return new_inst;
    };

    reg( rewind, range_vec( unsigned( rewind->cx.pos.size() ) ) );
    while ( ! pending_trans.empty() ) {
        PendingRewindTrans pt = pending_trans.back();
        pending_trans.pop_back();

        Vec<unsigned> keep_ind;
        Transition &t = pt.new_inst->orig->prev[ pt.num_prev ];
        for( unsigned keep_ind_next : pt.new_inst->orig_keed_ind )
            keep_ind.push_back_unique( t.rcitem[ keep_ind_next ] );

        // make the new instruction if not already done
        auto iter = instruction_map.find( RewindContext{ t.inst, keep_ind } );
        Instruction *new_inst = iter == instruction_map.end() ? reg( t.inst, keep_ind ) : iter->second;

        // std::cout << pt.new_inst->orig->get_display_id() << "\t->\t" << t.inst->get_display_id() << "\tki=[" << keep_ind << "]:\t" << new_inst->get_display_id() << "\t-> " << *new_inst << std::endl;

        // register in the graph
        new_inst->next_rw.secure_push_back( pt.num_next, pt.new_inst );
        pt.new_inst->prev << new_inst;

        //
        if ( t.inst == mark && keep_ind == rewind->cx.paths_to_mark.uniqued() )
            rewind->exec = new_inst;
    }

    // strings present in mark but not in rewind
    for( const auto &mp : mark->cx.paths_to_strings )
        if ( ! rewind->cx.paths_to_strings.count( mp.first ) )
            rewind->strs_to_cancel << mp.first;
}

} // namespace Hpipe

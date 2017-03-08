#include "InstructionTestContiguous.h"
#include "InstructionMultiCond.h"
#include "InstructionNextChar.h"
#include "InstructionRewind.h"
#include "InstructionAddStr.h"
#include "InstructionClrStr.h"
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
#include <set>

namespace Hpipe {

InstructionGraph::InstructionGraph( CharGraph *cg, const std::vector<std::string> &disp, int stop_char, bool disp_inst_pred, bool disp_trans_freq, bool want_boyer_moore, bool no_training ) : cg( cg ), init( 0 ), cx_ok( cg->char_item_ok, false ), no_training( no_training ) {
    // predefined instructions
    ok = inst_pool << new InstructionOK( cx_ok );
    ko = inst_pool << new InstructionKO( Context{} );
    cache[ cx_ok ] = ok;

    // first graph
    make_init();
    disp_if( disp, disp_inst_pred, disp_trans_freq, "init" );

    // rewinds
    make_mark_data( init, 0 );
    disp_if( disp, disp_inst_pred, disp_trans_freq, "mark" );

    // eq trans, eq pred (beware: rcitem becomes wrong)
    merge_eq_pred( init );
    disp_if( disp, disp_inst_pred, disp_trans_freq, "merge", false );

    // boyer-moore like optimizations
    if ( want_boyer_moore )
        boyer_moore();
    disp_if( disp, disp_inst_pred, disp_trans_freq, "boyer", false );

    // clean up
    remove_unused();
    disp_if( disp, disp_inst_pred, disp_trans_freq, "unused", false );

    // +1( MC, KO ) => +1[ no test ]( MC( 0 => KO, ... ) )
    // js, wo => 3.57
    // js, wi => 3.47
    // js, wi, prof => 2.71
    if ( stop_char >= 0 )
        opt_stop_char( stop_char );
    disp_if( disp, disp_inst_pred, disp_trans_freq, "stop_char", false );

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

    ++Instruction::cur_op_id;
    if ( init )
        init->write_dot_rec( os, disp_inst_pred, disp_trans_freq, disp_rc_item );

    os << "}\n";

    os.close();

    return exec_dot( f, prg );
}

Instruction *InstructionGraph::root() {
    return init;
}

void InstructionGraph::make_init() {
    // as much as possible, we try to move forward without cycles (to avoid miss of possible "free mark")
    std::queue<PendingTrans> pending_trans, pending_cycle_trans;
    pending_trans.emplace( nullptr, 0, Context( cg->root(), true ), Vec<unsigned>{} );
    while ( pending_trans.size() || pending_cycle_trans.size() ) {
        bool avoid_cycles = pending_trans.size();
        PendingTrans &pt = avoid_cycles ? pending_trans.front() : pending_cycle_trans.front();

        // get instruction and pending transitions
        Vec<PendingTrans> loc_pending_trans;
        Instruction *inst = pt.res ? pt.res : make_transitions( loc_pending_trans, pt.cx, avoid_cycles, pt );

        // if lead to a cycle, do the transition later
        if ( avoid_cycles && ! inst ) {
            pending_cycle_trans.emplace( pt );
            pending_trans.pop();
            continue;
        }

        // insert it in the graph
        if ( pt.inst ) {
            if ( pt.inst->next.size() <= pt.num_edge )
                pt.inst->next.resize( pt.num_edge + 1 );
            pt.inst->next[ pt.num_edge ] = { inst, pt.rcitem };
            inst->prev.emplace_back( pt.inst, pt.rcitem );
        } else
            init = inst;

        // for each transition to be completed, look it it can cancel the mark (in which case we may modify the pending marks)
        if ( pt.inst && pt.inst->mark ) {
            for( PendingTrans &npt : loc_pending_trans ) {
                if ( CharGraph::leads_to_ok( npt.cx.pos ) || npt.cx.pos.empty() ) {
                    if ( npt.cx.pos.size() == 1 && npt.cx.pos[ 0 ]->type == CharItem::OK ) {
                        std::set<std::pair<Instruction *,unsigned>> possible_instructions;
                        display_dot( cg );
                        PRINT( can_make_a_rewind( possible_instructions, pt.inst->mark, inst, npt.rcitem ) );
                    }

                    std::set<std::pair<Instruction *,unsigned>> possible_instructions;
                    if ( can_make_a_rewind( possible_instructions, pt.inst->mark, inst, npt.rcitem ) ) {
                        // we have the instruction for npt
                        InstructionRewind *rwnd = inst_pool << new InstructionRewind( npt.cx );
                        pt.inst->mark->rewinds << rwnd;
                        npt.res = rwnd;

                        // what to do after the rwnd
                        pending_trans.emplace( rwnd, 0, npt.cx.without_mark(), range_vec( unsigned( npt.rcitem.size() ) ) );
                    }
                }
            }
        }

        // reg loc_pending_trans
        ( avoid_cycles ? pending_trans : pending_cycle_trans ).pop();
        for( PendingTrans &pt : loc_pending_trans )
            pending_trans.push( pt );
    }
}

Instruction *InstructionGraph::make_transitions( Vec<PendingTrans> &pending_trans, const Context &cx, bool avoid_cycles, PendingTrans pt ) {
    std::map<Context,Instruction *>::iterator iter = cache.find( cx );
    if ( iter != cache.end() ) {
        // not a "cycles" phase (we first try to make straight paths, in order to avoid missing possible rewinds)
        if ( avoid_cycles )
            return 0;
        // making a cycle in a mark is ok, only if it does not change the rewinds we already have
        Instruction *inst = iter->second;
        if ( inst->mark ) {
            // adding a temporary link
            if ( pt.inst->next.size() <= pt.num_edge )
                pt.inst->next.resize( pt.num_edge + 1 );
            pt.inst->next[ pt.num_edge ] = { inst, pt.rcitem };
            inst->prev.emplace_back( pt.inst, pt.rcitem );

            // test if rewinds are still valid
            for( InstructionRewind *rewind : inst->mark->rewinds ) {
                std::set<std::pair<Instruction *,unsigned>> possible_instructions;
                if ( ! can_make_a_rewind( possible_instructions, inst->mark, rewind, range_vec<unsigned>( rewind->cx.pos.size() ) ) ) {
                    // remove the temporary link
                    inst->prev.pop_back();
                    return make_transitions( pending_trans, cx.with_new_num_path(), avoid_cycles, pt );
                }
            }

            // remove the temporary link (will be added again)
            inst->prev.pop_back();
        }
        return iter->second;
    }

    auto reg = [&]( auto *inst ) {
        cache.insert( iter, { cx, inst_pool << inst } );
        return inst;
    };
    auto tra = [&]( Instruction *inst, unsigned num_edge, const Context::PC &pc ) {
        pending_trans.emplace_back( inst, num_edge, pc.first, pc.second );
        return inst;
    };

    // dead end ?
    if ( cx.pos.empty() )
        return ko;

    // something to skip ?
    for( const CharItem *item : cx.pos )
        if ( item->type == CharItem::BEGIN or item->type == CharItem::PIVOT or item->type == CharItem::LABEL or item->type == CharItem::BEG_STR or item->type == CharItem::END_STR )
            return tra( reg( new InstructionSkip( cx ) ), 0, cx.forward( item ) );

    // everything is OK ?
    if ( cx.only_has( CharItem::OK ) )
        return ok;

    // EOF test ?
    for( const CharItem *i : cx.pos ) {
        if ( i->type == CharItem::_EOF ) {
            InstructionEof *res = reg( new InstructionEof( cx, cx.beg() ) );
            tra( res, 0, cx.with_not_eof().without( i ) );
            tra( res, 1, cx.with_eof    ().forward( i ) );
            return res;
        }
    }

    // we have a code ?
    for( unsigned ind = 0; ind < cx.pos.size(); ++ind ) {
        const CharItem *item = cx.pos[ ind ];
        if ( item->code_like() ) {
            // if it's too early to decide if this code can be executed, we have to add a mark
            if ( not cx.mark and ( cx.pos.size() >= 2 or not CharGraph::leads_to_ok( cx.pos ) ) ) {
                InstructionMark *res = reg( new InstructionMark( cx, ind ) );
                return tra( res, 0, cx.with_mark( res ) );
            }

            // Instruction type
            Instruction *res = 0;
            switch( item->type ) {
            case CharItem::CODE:    res = reg( new InstructionCode  ( cx, item->str, ind ) ); break;
            case CharItem::ADD_STR: res = reg( new InstructionAddStr( cx, item->str, ind ) ); break;
            case CharItem::CLR_STR: res = reg( new InstructionClrStr( cx, item->str, ind ) ); break;
            default: HPIPE_TODO;
            }

            //
            if ( not cx.mark and cx.beg() and res->data_code() )
                cg->err( "A code cannot correctly depend on a char data before a first char has been queried" );

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
            Context ncx_ok( cx.mark, cx.flags );
            ncx_ok.pos << cg->char_item_ok;
            return tra( reg( new InstructionSkip( cx ) ), 0, Context::PC{ ncx_ok, first_leads_to_ok } );
        }
        if ( cx.has( CharItem::OK ) )
            return tra( new InstructionSkip( cx ), 0, cx.only_with( CharItem::OK ) );
        return tra( reg( new InstructionSkip( cx ) ), 0, Context::PC{ {}, {} } );
    }

    Instruction *res = reg( new InstructionNextChar( cx, cx.beg() ) );
    tra( res, 0, cx.without_beg().without_not_eof().forward( CharItem::NEXT_CHAR ) ); // if OK

    if ( not cx.not_eof() ) {
        if ( first_leads_to_ok >= 0 ) { // result will be an OK instruction, with an edge enabling a rewind
            Context ncx_ok( cx.mark, cx.flags | Context::ON_EOF );
            ncx_ok.pos << cg->char_item_ok;
            tra( res, 1, Context::PC{ ncx_ok, first_leads_to_ok } ); // if no next char
        } else
            tra( res, 1, cx.with_eof().only_with( CharItem::OK ) ); // if no next char
    }
    return res;
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

void InstructionGraph::remove_unused() {
    // list of instructions that can be removed from the graph
    Vec<Instruction *> to_remove;
    ++Instruction::cur_op_id;
    init->get_unused_rec( to_remove, init );

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
        bool same_ko = true;
        InstructionNextChar *fc = dynamic_cast<InstructionNextChar *>( mc->prev[ 0 ].inst );
        Instruction *ko = fc->next.size() >= 2 ? fc->next[ 1 ].inst : 0;
        for( int i = 0; i < mc->prev.size(); ++i ) {
            InstructionNextChar *nc = dynamic_cast<InstructionNextChar *>( mc->prev[ i ].inst );
            if ( nc == 0 || nc->next[ 0 ].inst != mc || ( ko ? nc->next[ 1 ].inst != ko : nc->next.size() < 2 ) ) {
                same_ko = false;
                break;
            }
        }
        if( ! same_ko )
            continue;

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
    
    unsigned nb_conds = 0;
    root()->apply( [&]( Instruction *inst ) {
        nb_conds += dynamic_cast<InstructionMultiCond *>( inst ) != 0;
    } );
    std::cerr << "Nb conds to optimize: " << nb_conds << std::endl;

    ++Instruction::cur_op_id;
    init->optimize_conditions( inst_pool );
}

void InstructionGraph::merge_eq_pred( Instruction *&root ) {
    // get sub graphs
    Vec<InstructionRewind *> rewinds;
    root->apply( [&]( Instruction *inst ) {
        if ( InstructionRewind *rw = dynamic_cast<InstructionRewind *>( inst ) )
            rewinds << rw;
    } );

    // merge_eq_pred in sub graphs
    for( InstructionRewind *rw : rewinds )
        if ( rw->exec )
            merge_eq_pred( rw->exec );

    // set inst->_cache_code_repr to avoid ++Instruction::cur_op_id inside the find_rec
    root->apply( []( Instruction *inst ) {
        inst->code_repr();
    } );

    //
    while ( true ) {
        ++Instruction::cur_op_id;
        if ( not root->find_rec( [ &root ]( Instruction *inst ) { return inst->merge_predecessors( &root ); } ) )
            break;
    }

    root->apply( [&]( Instruction *inst ) {
        inst->merge_eq_next( inst_pool );
    } );
}

void InstructionGraph::boyer_moore() {
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

Instruction *InstructionGraph::make_rewind_inst( Vec<PendingRewindTrans> &pending_trans, std::map<InstructionGraph::RewindContext, Instruction *> &instruction_map, std::unordered_map<Instruction *, Vec<unsigned> > possible_inst, InstructionRewind *rewind, Instruction *orig, const PendingRewindTrans &pt, bool avoid_cycle, bool force_copy ) {
    std::map<RewindContext,Instruction *>::iterator iter = instruction_map.find( RewindContext{ orig, pt.rewind_mark } );
    if ( force_copy == false && iter != instruction_map.end() ) {
        // not a "cycles" phase (we first try to make straight paths, in order to avoid missing possible rewinds)
        if ( avoid_cycle )
            return 0;

        //
        Instruction *inst = iter->second;
        if ( inst->mark ) {
            Transition &t = pt.inst->orig->next[ pt.num_trans ];
            const Vec<unsigned> &keep_ind_curr = possible_inst[ pt.inst->orig ];
            const Vec<unsigned> &keep_ind_next = possible_inst[ t.inst ];

            // adding of a temporary link
            Vec<unsigned> nrcitem;
            if ( pt.use_rv ) {
                nrcitem = range_vec( unsigned( keep_ind_next.size() ) );
            } else {
                Vec<unsigned> corr_nrcitem;
                corr_nrcitem.resize( pt.inst->orig->cx.pos.size() );
                for( unsigned i = 0; i < keep_ind_curr.size(); ++i )
                    corr_nrcitem[ keep_ind_curr[ i ] ] = i;

                nrcitem.reserve( keep_ind_next.size() );
                for( unsigned ind : keep_ind_next )
                    nrcitem << corr_nrcitem[ t.rcitem[ ind ] ];
            }

            if ( pt.inst->next.size() <= pt.num_edge )
                pt.inst->next.resize( pt.num_edge + 1 );
            pt.inst->next[ pt.num_edge ] = { inst, nrcitem };
            inst->prev.emplace_back( pt.inst, nrcitem );

            // test if rewinds for inst->mark are still valid
            for( InstructionRewind *rewind : inst->mark->rewinds ) {
                std::set<std::pair<Instruction *,unsigned>> possible_instructions;
                if ( ! can_make_a_rewind( possible_instructions, inst->mark, rewind, range_vec<unsigned>( rewind->cx.pos.size() ) ) ) {
                    // remove the temporary link and return a copy
                    inst->prev.pop_back();
                    return make_rewind_inst( pending_trans, instruction_map, possible_inst, rewind, orig, pt, avoid_cycle, true );
                }
            }

            // remove the temporary link
            inst->prev.pop_back();
        }

        return iter->second;
    }

    Context cx( pt.rewind_mark, false );
    const Vec<unsigned> &keep_ind = possible_inst.find( orig )->second;
    for( unsigned ind : keep_ind )
        cx.pos << orig->cx.pos[ ind ];

    Instruction *res;
    if ( orig == rewind ) {
        res = inst_pool << new InstructionOK( cx );
    } else {
        Vec<unsigned> ind_keeped_instr; ///< indices of keeped transitions
        for( unsigned ind = 0; ind < orig->next.size(); ++ind )
            if ( possible_inst.count( orig->next[ ind ].inst ) )
                ind_keeped_instr << ind;

        res = orig->clone( inst_pool, cx, ind_keeped_instr );

        // need a mark ?
        if ( InstructionWithCode *code = dynamic_cast<InstructionWithCode *>( res ) ) {
            if ( cx.pos.size() > 1 and not pt.rewind_mark ) {
                InstructionMark *mark = inst_pool << new InstructionMark( cx, keep_ind.index_first( code->num_active_item ) );
                instruction_map.insert( iter, { RewindContext{ orig, pt.rewind_mark }, mark } );
                mark->orig = pt.inst->orig;

                pending_trans.emplace_back( mark, 0, pt.num_trans, mark, nullptr, true );
                return mark;
            }
        }

        // next instructions
        for( unsigned ind = 0, num = 0; ind < orig->next.size(); ++ind )
            if ( possible_inst.count( orig->next[ ind ].inst ) )
                pending_trans.emplace_back( res, num++, ind, pt.rewind_mark );
    }

    instruction_map.insert( iter, { RewindContext{ orig, pt.rewind_mark }, res } );
    res->orig = orig;
    return res;
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

bool InstructionGraph::can_make_a_rewind( std::set<std::pair<Instruction *,unsigned>> &possible_instructions, InstructionMark *mark, Instruction *inst, Vec<unsigned> rcitem ) {
    if ( rcitem.empty() )
        return true;

    // check that the ambiguity has disappeared at least on the first instruction after the mark
    for( unsigned ind : rcitem )
        get_possible_inst_rec( possible_instructions, inst, ind, mark );

    //
    #ifdef TEST_ONLY_FIRST_IN_CAN_MAKE_A_REWIND
    bool has_active = false;
    bool has_inactive = false;
    for( unsigned ind = 0; ind < mark->cx.pos.size(); ++ind )
        if ( possible_instructions.count( { mark, ind } ) )
            ( ind == mark->num_active_item ? has_active : has_inactive ) = true;
    return has_active ^ has_inactive;
    #else
    std::set<InstructionWithCode *> possible_with_code;
    for( auto &p : possible_instructions )
        if ( InstructionWithCode *code = dynamic_cast<InstructionWithCode *>( p.first ) )
            possible_with_code.insert( code );
    for( InstructionWithCode *code : possible_with_code ) {
        bool has_active = false;
        bool has_inactive = false;
        for( unsigned ind = 0; ind < code->cx.pos.size(); ++ind )
            if ( possible_instructions.count( { code, ind } ) )
                ( ind == code->num_active_item ? has_active : has_inactive ) = true;
        if ( has_active && has_inactive )
            return false;
    }
    return true;
    #endif
}


void InstructionGraph::get_possible_inst_rec( std::set<std::pair<Instruction *,unsigned>> &possible_instructions, Instruction *inst, unsigned pos, const InstructionMark *mark ) {
    auto iter = possible_instructions.find( { inst, pos } );
    if ( iter != possible_instructions.end() )
        return;
    possible_instructions.insert( iter, { inst, pos } );

    if ( inst != mark )
        for( const Transition &p : inst->prev )
            get_possible_inst_rec( possible_instructions, p.inst, p.rcitem[ pos ], mark );
}

void remove_mark_rec( Instruction *inst, Instruction *mark ) {
    if ( inst->op_id == Instruction::cur_op_id )
        return;
    inst->op_id = Instruction::cur_op_id;

    if ( not inst->with_code() )
        inst->mark = 0;

    for( Transition &p : inst->prev )
        if ( p.inst != mark )
            remove_mark_rec( p.inst, mark );
}

void InstructionGraph::make_mark_data( Instruction *root, int rec_level ) {
    if ( rec_level == 16 ) {
        cg->err( "Max rec level exceeded in make_mark_data. Possible internal hpipe problem." );
        return;
    }

    // get the marks
    ++Instruction::cur_op_id;
    Vec<InstructionMark *> marks;
    root->apply( [&]( Instruction *inst ) {
        if ( InstructionMark *mark = dynamic_cast<InstructionMark *>( inst ) )
            marks << mark;
    } );

    // make rewind->exec (to be done in a first phase, before modifications of the graph)
    for( InstructionMark *mark : marks )
        for( InstructionRewind *rewind : mark->rewinds )
            make_rewind_exec( mark, rewind, rec_level );

    // add save point, remove mark if possible
    for( InstructionMark *mark : marks ) {
        // make rewind->exec, update flags for mark
        bool remove_mark = true;
        unsigned num_save = 0;
        for( InstructionRewind *rewind : mark->rewinds ) {
            if ( rewind->need_rw )
                remove_mark = false;
            else if ( rewind->code_seq.size() ) {
                // add save points
                for( InstructionWithCode *code : rewind->code_seq ) {
                    if ( not code->data_code() )
                        continue;
                    Instruction *orig = code->orig;
                    while ( orig->prev.size() == 1 and dynamic_cast<InstructionWithCode *>( orig->prev[ 0 ].inst ) )
                        orig = orig->prev[ 0 ].inst;
                    if ( orig->prev.size() == 1 and dynamic_cast<InstructionSave *>( orig->prev[ 0 ].inst ) ) {
                        code->save = static_cast<InstructionSave *>( orig->prev[ 0 ].inst );
                        continue;
                    }
                    code->save = inst_pool << new InstructionSave( orig->cx, num_save++ );
                    orig->insert_before_this( code->save, init );
                    code->save->mark = mark;
                }
            }
        }

        //
        if ( remove_mark ) {
            ++Instruction::cur_op_id;
            for( InstructionRewind *rewind : mark->rewinds ) {
                remove_mark_rec( rewind, mark );
                if ( rewind->code_seq.empty() )
                    rewind->remove();
                else
                    rewind->mark = 0;
            }
            mark->remove();
        }
    }
}

void InstructionGraph::make_rewind_exec( InstructionMark *mark, InstructionRewind *rewind, int rec_level ) {
    if ( rewind->cx.pos.empty() )
        return;

    // get possible instructions/pos pairs
    std::set<std::pair<Instruction *,unsigned>> possible_inst_pos;
    get_possible_inst_rec( possible_inst_pos, rewind, 0, mark );

    // get possible instructions with possible CharItems
    std::unordered_map<Instruction *,Vec<unsigned>> possible_inst;
    for( const std::pair<Instruction *,unsigned> &p : possible_inst_pos )
        possible_inst[ p.first ] << p.second;

    // corr for mark (the first inst of the rewind->exec graph)
    Context cx;
    for( unsigned ind : possible_inst[ mark ] )
        cx.pos << mark->cx.pos[ ind ];
    rewind->exec = inst_pool << new InstructionNone( cx );
    rewind->exec->orig = mark;

    // clone instructions
    std::queue<PendingRewindTrans> pending_trans, pending_cycle_trans;
    std::map<RewindContext,Instruction *> instruction_map;
    for( unsigned num_trans = 0, num = 0; num_trans < mark->next.size(); ++num_trans )
        if ( possible_inst.count( mark->next[ num_trans ].inst ) )
            pending_trans.emplace( rewind->exec, num++, num_trans, nullptr );
    while ( ! pending_trans.empty() || ! pending_cycle_trans.empty() ) {
        bool avoid_cycles = pending_trans.size();
        PendingRewindTrans &pt = avoid_cycles ? pending_trans.front() : pending_cycle_trans.front();
        Transition &t = pt.inst->orig->next[ pt.num_trans ];

        Vec<PendingRewindTrans> loc_pending_trans;
        const Vec<unsigned> &keep_ind_curr = possible_inst[ pt.inst->orig ];
        const Vec<unsigned> &keep_ind_next = possible_inst[ t.inst ];
        Instruction *inst = pt.res ? pt.res : make_rewind_inst( loc_pending_trans, instruction_map, possible_inst, rewind, t.inst, pt, avoid_cycles, false );

        if ( avoid_cycles && ! inst ) {
            pending_cycle_trans.emplace( pt );
            pending_trans.pop();
            continue;
        }

        // insert it in the graph
        Vec<unsigned> nrcitem;
        if ( pt.use_rv ) {
            nrcitem = range_vec( unsigned( keep_ind_next.size() ) );
        } else {
            Vec<unsigned> corr_nrcitem;
            corr_nrcitem.resize( pt.inst->orig->cx.pos.size() );
            for( unsigned i = 0; i < keep_ind_curr.size(); ++i )
                corr_nrcitem[ keep_ind_curr[ i ] ] = i;

            nrcitem.reserve( keep_ind_next.size() );
            for( unsigned ind : keep_ind_next )
                nrcitem << corr_nrcitem[ t.rcitem[ ind ] ];
        }

        if ( pt.inst->next.size() <= pt.num_edge )
            pt.inst->next.resize( pt.num_edge + 1 );
        pt.inst->next[ pt.num_edge ] = { inst, nrcitem };
        inst->prev.emplace_back( pt.inst, nrcitem );

        // for each new transition to be completed, look if it is an opportunity to cancel the mark (in which case we may modify the pending marks)
        if ( inst->mark ) {
            for( PendingRewindTrans &npt : loc_pending_trans ) {
                Transition &nt = npt.inst->orig->next[ npt.num_trans ];

                auto can_add_a_rewind = [&]() {
                    // npt.inst to upcoming (that will be cloned from nt.inst)
                    Vec<unsigned> nnrcitem;
                    if ( npt.use_rv ) {
                        nnrcitem = range_vec( unsigned( npt.inst->cx.pos.size() ) );
                    } else {
                        const Vec<unsigned> &keep_ind_0 = possible_inst[ npt.inst->orig ];
                        const Vec<unsigned> &keep_ind_1 = possible_inst[ nt.inst ];

                        Vec<unsigned> corr_nnrcitem;
                        corr_nnrcitem.resize( npt.inst->orig->cx.pos.size(), 0 );
                        for( unsigned i = 0; i < keep_ind_0.size(); ++i )
                            corr_nnrcitem[ keep_ind_0[ i ] ] = i;

                        nnrcitem.reserve( keep_ind_1.size() );
                        for( unsigned ind : keep_ind_1 )
                            nnrcitem << corr_nnrcitem[ nt.rcitem[ ind ] ];
                    }

                    // check that the ambiguity has disappeared
                    std::set<std::pair<Instruction *,unsigned>> possible_instructions;
                    for( unsigned ind : nnrcitem )
                        get_possible_inst_rec( possible_instructions, npt.inst, ind, inst->mark );

                    bool has_active = false;
                    bool has_inactive = false;
                    for( unsigned ind = 0; ind < inst->mark->cx.pos.size(); ++ind )
                        if ( possible_instructions.count( { inst->mark, ind } ) )
                            ( ind == inst->mark->num_active_item ? has_active : has_inactive ) = true;

                    return has_active ^ has_inactive;
                };

                if ( can_add_a_rewind() ) {
                    //
                    Context cx( npt.rewind_mark, false );
                    for( unsigned ind : possible_inst[ nt.inst ] )
                        cx.pos << nt.inst->cx.pos[ ind ];

                    // we have the instruction for pt
                    InstructionRewind *rwnd = inst_pool << new InstructionRewind( cx );
                    inst->mark->rewinds << rwnd;
                    rwnd->orig = npt.inst->orig;
                    npt.res = rwnd;

                    // what to do after the rwnd (continue without the mark, and with a range_vec for the vector)
                    pending_trans.emplace( rwnd, 0, npt.num_trans, nullptr, nullptr, true );
                    //npt.use_rv = true;
                    //npt.inst = rwnd;
                }
            }
        }

        // reg loc_pending_trans
        ( avoid_cycles ? pending_trans : pending_cycle_trans ).pop();
        for( PendingRewindTrans &npt : loc_pending_trans )
            pending_trans.push( npt );
    }

    // make marks in subgraph if necessary
    make_mark_data( rewind->exec, rec_level + 1 );

    // information needed for simplifications
    rewind->exec->update_in_a_branch();
    rewind->exec->update_in_a_cycle();

    rewind->exec->apply( [&]( Instruction *inst ) {
        if ( InstructionWithCode *code = dynamic_cast<InstructionWithCode *>( inst ) ) {
            rewind->code_seq << code;
            if ( inst->in_a_cycle || inst->in_a_branch )
                rewind->need_rw = true;
        } else if ( InstructionRewind *rw = dynamic_cast<InstructionRewind *>( inst ) ) {
            if ( rw->code_seq.size() ) {
                if ( rw->in_a_cycle || rw->in_a_branch || rw->need_rw )
                    rewind->need_rw = true;
                else
                    for( InstructionWithCode *code : rw->code_seq )
                        rewind->code_seq << static_cast<InstructionWithCode *>( code->orig );
            }
        }
    } );
}

} // namespace Hpipe

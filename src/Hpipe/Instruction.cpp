#include "InstructionNextChar.h"
#include "CppEmitter.h"
#include "CharGraph.h"
#include "DotOut.h"
#include "Assert.h"

namespace Hpipe {

unsigned Instruction::cur_op_id = 0;

Instruction::Instruction( Context cx ) : cx( cx ), display_id( 0 ), op_id( 0 ) {
    in_a_cycle   = false;
    num_ordering = -2;
    id_gen       = 0;
    mark         = cx.mark;
    orig         = nullptr;
    cum_freq     = 0;
}

Instruction::~Instruction() {
}

void Instruction::write_to_stream( std::ostream &os ) const {
    os << "(" << get_display_id() << ") ";
    write_dot( os );
}

void Instruction::write_dot_add( std::ostream &os, bool disp_inst_pred, bool disp_trans_freq, bool disp_rc_item ) const {
}

void Instruction::write_dot_rec( std::ostream &os, bool disp_inst_pred, bool disp_trans_freq, bool disp_rc_item, bool rec ) const {
    if ( op_id == cur_op_id )
        return;
    op_id = cur_op_id;

    os << "  node_" << this << " [label=\"";

    std::ostringstream ss;

    if ( display_id )
        ss << "(" << display_id << ") ";

    if ( disp_rc_item ) {
        ss << "[";
        for( unsigned i = 0; i < cx.pos.size(); ++i )
            ss << ( i ? "," : "" ) << cx.pos[ i ]->compact_repr();
        ss << "]";
        if ( cx.flags ) {
            ss << "(";
            if ( cx.flags & cx.FL_BEG     ) ss << "B";
            if ( cx.flags & cx.FL_EOF     ) ss << "O";
            if ( cx.flags & cx.FL_NOT_EOF ) ss << "N";
            if ( cx.flags & cx.FL_OK      ) ss << "K";
            ss << ")";
        }
        ss << " ";
    }

    std::vector<std::string> edge_labels;
    write_dot( ss, &edge_labels );

    if ( mark ) {
        ss << "\n";
        for( bool v : cx.code_path )
            ss << v;
    }

    for( std::string str : running_str )
        ss << "\nrs: " << str;

    // ss << "in=" << cx.in << " ";

    dot_out( os, ss.str() );

    os << "\"";
    if ( in_a_cycle )
        os << ",style=dotted";
    if ( mark )
        os << ",color=green";
    os << "];\n";

    write_dot_add( os, disp_inst_pred, disp_trans_freq, disp_rc_item );

    for( unsigned nt = 0; nt < next.size(); ++nt) {
        const Transition &t = next[ nt ];
        if ( rec && t.inst )
            t.inst->write_dot_rec( os, disp_inst_pred, disp_trans_freq, disp_rc_item, rec );

        std::ostringstream label;
        int cpt = 0;
        if ( disp_rc_item )
            label << ( cpt++ ? "\n" : "" ) << t.rcitem;
        if ( disp_trans_freq and t.freq >= 0 )
            label << ( cpt++ ? "\n" : "" ) << "f=" << t.freq;
        if ( nt < edge_labels.size() )
            label << ( cpt++ ? "\n" : "" ) << edge_labels[ nt ];

        dot_out( os << "  node_" << this << " -> node_" << t.inst << " [label=\"", label.str() ) << "\"";

        //        switch ( nt ) {
        //        case 0:  break;
        //        case 1:  os << ",style=dashed"; break;
        //        default: os << ",style=dotted"; break;
        //        }

        if ( nt and dynamic_cast<const InstructionNextChar *>( this ) )
            os << ",color=lightgray";

        os << "];\n";
    }

    if ( disp_inst_pred ) {
        for( const Transition &t : prev ) {
            std::ostringstream label;
            int cpt = 0;
            //            if ( t.inst->type == Instruction::COND ) {
            //                if ( t.eof )
            //                    label << ( cpt++ ? "\n" : "" ) << "EOF";
            //                if ( not t.cond.never_checked() or t.eof == false )
            //                    label << ( cpt++ ? "\n" : "" ) << t.cond;
            //            }

            if ( disp_rc_item )
                label << ( cpt++ ? "\n" : "" ) << t.rcitem;

            dot_out( os << "  node_" << t.inst << " -> node_" << this << " [label=\"", label.str() ) << "\",color=blue];\n";
        }
    }
}

unsigned Instruction::get_display_id() const {
    if ( not display_id ) {
        static unsigned cur_display_id = 0;
        display_id = ++cur_display_id;
    }
    return display_id;
}

void Instruction::apply( std::function<void (Instruction *)> f, bool subgraphs ) {
    ++cur_op_id;
    apply_rec( f, subgraphs );
}

void Instruction::apply_rec( std::function<void (Instruction *)> f, bool subgraphs ) {
    if ( op_id == Instruction::cur_op_id )
        return;
    op_id = Instruction::cur_op_id;

    f( this );

    for( Transition &t : next )
        t.inst->apply_rec( f, subgraphs );
}

void Instruction::get_unused_rec( Vec<Instruction *> &to_remove, Instruction *&init ) {
    if ( op_id == Instruction::cur_op_id )
        return;
    op_id = Instruction::cur_op_id;

    if ( can_be_deleted() ) {
        if ( this == init ) {
            if ( next.size() == 1 ) {
                init = next[ 0 ].inst;
                to_remove << this;
            }
        } else
            to_remove << this;
    }

    for( Transition &t : next )
        t.inst->get_unused_rec( to_remove, init );
}

bool Instruction::find_rec( std::function<bool (Instruction *)> f ) {
    if ( op_id == Instruction::cur_op_id )
        return false;
    op_id = Instruction::cur_op_id;

    if ( f( this ) )
        return true;

    for( Transition &t : next )
        if ( t.inst->find_rec( f ) )
            return true;
    return false;
}

void Instruction::apply_rec_rewind_l( std::function<void (Instruction *, unsigned)> f, unsigned rewind_level ) {
    if ( op_id == Instruction::cur_op_id )
        return;
    op_id = Instruction::cur_op_id;

    f( this, rewind_level );

    for( Transition &t : next )
        t.inst->apply_rec_rewind_l( f, rewind_level );
}

Transition *Instruction::train( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous ) {
    if ( next.empty() )
        return 0;
    if ( next.size() != 1 )
        PRINTLE( *this ); // need a surdef
    //     HPIPE_ASSERT( next.size() == 1, "" );
    return &next[ 0 ];
}

void Instruction::train_rec( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous ) {
    for( Instruction *inst = this; inst; ) {
        if ( s >= 0 and s < inp.size() )
            inst->freq[ inp[ s ] ] += freq; // wrong: do not use off_data
        inst->cum_freq += freq;

        Transition *trans = inst->train( s, m, inp, freq, use_contiguous );
        if ( not trans )
            break;
        trans->freq += freq;
        inst = trans->inst;
    }
}

bool Instruction::can_be_deleted() const {
    return false;
}

bool Instruction::is_a_mark() const {
    return false;
}

int Instruction::save_in_loc_reg() const {
    return -1;
}

void Instruction::remove( bool update_rcitem ) {
    for( Transition &p : prev ) {
        HPIPE_ASSERT( p.inst != this, "..." );

        unsigned ind = p.inst->next.index_first_checking( [&]( Transition &t ) {
                return t.inst == this;
    } );
        p.inst->next.remove( ind );
        for( Transition &t : next ) {
            Vec<unsigned> rcitem;
            if ( update_rcitem ) {
                rcitem.reserve( t.rcitem.size() );
                for( unsigned ind : t.rcitem )
                    rcitem << p.rcitem[ ind ];
            }

            p.inst->next.insert( p.inst->next.begin() + ind, Transition( t.inst, rcitem, t.freq ) );
        }
    }

    for( Transition &t : next ) {
        HPIPE_ASSERT( t.inst != this, "..." );

        unsigned ind = t.inst->prev.index_first_checking( [&]( Transition &p ) {
            return p.inst == this;
        } );
        t.inst->prev.remove( ind );
        for( Transition &p : prev ) {
            Vec<unsigned> rcitem;
            if ( update_rcitem ) {
                rcitem.reserve( t.rcitem.size() );
                for( unsigned ind : t.rcitem )
                    rcitem << p.rcitem[ ind ];
            }

            t.inst->prev.insert( t.inst->prev.begin() + ind, Transition( p.inst, rcitem ) );
        }
    }
}

void Instruction::insert_before_this( Instruction *inst, Instruction *&init ) {
    if ( init == this )
        init = inst;

    // p.inst -[a b c]-> this
    // p.inst -[a b c]-> inst -[0 1 2]-> this
    for( Transition &p : prev ) {
        inst->prev << p;
        for( Transition &t : p.inst->next ) {
            if ( t.inst == this and p.rcitem == t.rcitem ) {
                t.inst = inst;
                break;
            }
        }
    }
    prev.clear();
    prev << Transition( inst, range_vec( unsigned( cx.pos.size() ) ) );
    inst->next << Transition( this, range_vec( unsigned( cx.pos.size() ) ) );
}

void Instruction::repl_in_preds( Instruction *inst ) {
    for( Transition &p : prev ) {
        for( Transition &t : p.inst->next ) {
            if ( t.inst == this ) {
                t.inst = inst;
                break;
            }
        }
        inst->prev << Transition( p.inst, p.rcitem );
    }
    prev.clear();
}

void Instruction::reg_var( std::function<void(std::string,std::string)> f, Hpipe::CppEmitter *cpp_emitter ) {
}

void Instruction::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    ss << "TODO; // " << *this;
    PRINTLE( "write_cpp", *this );
    //TODO;
}

int Instruction::get_id_gen( CppEmitter *cpp_emitter ) {
    if ( not id_gen )
        id_gen = ++cpp_emitter->nb_id_gen;
    return id_gen;

}

void Instruction::write_trans( StreamSepMaker &ss, CppEmitter *cpp_emitter, unsigned num ) {
    if ( next[ num ].inst->num_ordering != num_ordering + 1 )
        ss << "goto l_" << next[ num ].inst->get_id_gen( cpp_emitter ) << ";";
}

void Instruction::optimize_conditions( PtrPool<Instruction> &inst_pool ) {
    if ( op_id == Instruction::cur_op_id )
        return;
    op_id = Instruction::cur_op_id;

    for( Transition &t : next )
        t.inst->optimize_conditions( inst_pool );
}

void Instruction::find_cond_leaves( std::map<Instruction *,Cond> &leaves, const Cond &in, Instruction *orig ) {
    if ( prev.size() == 1 and next.size() == 1 and can_be_deleted() )
        next[ 0 ].inst->find_cond_leaves( leaves, in, this );
    else {
        prev.remove_first_checking( [&]( Transition &p ) { return p.inst == orig; } );
        leaves[ this ] |= in;
    }
}

bool Instruction::with_code() const {
    return false;
}

bool Instruction::data_code() const {
    return false;
}

void Instruction::update_in_a_cycle() {
    ++Instruction::cur_op_id;
    Vec<std::pair<Instruction *,unsigned>> stack;
    stack.emplace_back( this, 0 );
    while ( true ) {
        std::pair<Instruction *,unsigned> t = stack.back();

        // instruction
        if ( t.first->op_id == Instruction::cur_op_id ) {
            stack.pop_back();
            if ( t.first->num_in_dfs_stack >= 0 )
                for( unsigned j = t.first->num_in_dfs_stack; j < stack.size(); ++j )
                    stack[ j ].first->in_a_cycle = true;
        } else {
            t.first->op_id = Instruction::cur_op_id;
            t.first->num_in_dfs_stack = stack.size() - 1;
        }

        // next
        while ( stack.back().second >= stack.back().first->next.size() ) {
            stack.back().first->num_in_dfs_stack = - 1;
            stack.pop_back();
            if ( stack.empty() )
                return;
        }
        stack.emplace_back( stack.back().first->next[ stack.back().second++ ].inst, 0 );
    }
}

void Instruction::update_in_a_branch() {
    Vec<Instruction *> tails;
    apply( [&]( Instruction *inst ) {
        inst->in_a_branch = true;
        if ( inst->next.empty() )
            tails << inst;
    } );

    // tails
    for( Instruction *inst : tails ) {
        while( true ) {
            if ( inst->in_a_branch == false )
                break;
            inst->in_a_branch = false;
            if ( inst->prev.size() != 1 )
                break;
            inst = inst->prev[ 0 ].inst;
        }
    }

    // head
    for( Instruction *inst = this; ; ) {
        if ( inst->in_a_branch == false )
            break;
        inst->in_a_branch = false;
        if ( inst->next.size() != 1 )
            break;
        inst = inst->next[ 0 ].inst;
    }
}

//void Instruction::get_boyer_moore_seq( Vec<std::pair<Vec<Cond>,Instruction *>> &front ) {
//    if ( next.size() == 0 )
//        return;
//    ++Instruction::cur_op_id;
//    Vec<std::pair<Instruction *,unsigned>> stack;
//    stack.emplace_back( this, 0 );

//    while ( true ) {
//        std::pair<Instruction *,unsigned> t = stack.back();

//        auto reg_stack = [&]() {
//            std::pair<Vec<Cond>,Instruction *> &vi = *front.new_elem();
//            Cond cond( 0, 255 );
//            for( std::pair<Instruction *,unsigned> p : stack ) {
//                if ( dynamic_cast<InstructionNextChar *>( p.first ) ) {
//                    vi.first << cond;
//                    cond = { 0, 255 };
//                } else if ( InstructionCond *inst = dynamic_cast<InstructionCond *>( p.first ) ) {
//                    if ( p.second == 1 )
//                        cond &= inst->cond;
//                    else
//                        cond &= ~inst->cond;
//                } else if ( InstructionMultiCond *inst = dynamic_cast<InstructionMultiCond *>( p.first ) ) {
//                    PRINT( inst );
//                    TODO;
//                }
//                vi.second = p.first;
//            }
//            vi.first << cond;
//        };

//        auto stop_inst = []( Instruction *inst ) {
//            return inst->with_code() or
//                   dynamic_cast<InstructionOK *> ( inst ) or
//                   dynamic_cast<InstructionKO *> ( inst );
//        };

//        // stop ?
//        if ( stop_inst( t.first ) or ( t.first->op_id == Instruction::cur_op_id and t.first->num_in_dfs_stack >= 0 ) ) {
//            reg_stack();
//            stack.pop_back();
//            if ( stack.empty() )
//                return;
//        } else {
//            t.first->op_id = Instruction::cur_op_id;
//            t.first->num_in_dfs_stack = stack.size() - 1;
//        }

//        // next
//        auto max = []( Instruction *inst ) -> unsigned {
//            if ( dynamic_cast<InstructionEof *>( inst ) or dynamic_cast<InstructionNextChar *>( inst ) )
//                return 1;
//            return inst->next.size();
//        };
//        while ( stack.back().second >= max( stack.back().first ) ) {
//            stack.back().first->num_in_dfs_stack = - 1;
//            stack.pop_back();
//            if ( stack.empty() )
//                return;
//        }
//        stack.emplace_back( stack.back().first->next[ stack.back().second++ ].inst, 0 );
//    }
//}

void Instruction::write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter, std::string repl_data, std::string repl_buf ) {
    PRINTLE( "save_seq", *this );
    HPIPE_TODO;
}

static bool can_be_merged( const Instruction *a, const Instruction *b ) {
    // check the transitions
    if ( a->next.size() != b->next.size() )
        return false;
    for( unsigned i = 0; i < a->next.size(); ++i )
        if ( a->next[ i ].inst != b->next[ i ].inst )
            return false;

    // check instruction
    return const_cast<Instruction *>( a )->code_repr() == const_cast<Instruction *>( b )->code_repr();
}


///
static void merge( Instruction **init, const Vec<Instruction *> &l ) {
    if ( not l.size() )
        return;

    // predecessors
    for( unsigned n = 1; n < l.size(); ++n ) {
        for( const Transition &p : l[ n ]->prev ) {
            l[ 0 ]->prev << Transition( p.inst, p.rcitem );
            for( Transition &t : p.inst->next ) {
                if ( t.inst == l[ n ] ) {
                    t.inst = l[ 0 ];
                    break;
                }
            }
        }
    }

    // transitions
    for( unsigned n = 1; n < l.size(); ++n ) {
        for( const Transition &t : l[ n ]->next ) {
            t.inst->prev.remove_first_checking( [&]( const Transition &p ) {
                return p.inst == l[ n ];
            } );
        }
    }

    //
    if ( init and l.contains( *init ) )
        *init = l[ 0 ];

    // simplifications
    //    for( Transition &pp : res->predecessors )
    //        pp.inst->merge_transitions( false );
    //    res->merge_transitions( false );
    // res->merge_predecessors( init );
}

bool Instruction::merge_predecessors( Instruction **init ) {
    for( unsigned i = 0; i < prev.size(); ++i ) {
        for( unsigned j = i + 1; j < prev.size(); ++j ) {
            if ( prev[ i ].inst != prev[ j ].inst and can_be_merged( prev[ i ].inst, prev[ j ].inst ) ) {
                merge( init, { prev[ i ].inst, prev[ j ].inst } );
                return true;
            }
        }
    }
    return false;
}

std::string Instruction::code_repr() {
    if ( _cache_code_repr.empty() ) {
        std::ostringstream ss;
        get_code_repr( ss );
        _cache_code_repr = ss.str();
    }
    return _cache_code_repr;
}

void Instruction::boyer_moore_opt( PtrPool<Instruction> &inst_pool, Instruction **init ) {
    if ( op_id == Instruction::cur_op_id )
        return;
    op_id = Instruction::cur_op_id;

    for( Transition &t : next )
        t.inst->boyer_moore_opt( inst_pool, init );
}

void Instruction::merge_eq_next( PtrPool<Instruction> &inst_pool ) {
}

bool Instruction::need_buf_next() const {
    return mark || ! running_str.empty();
}

bool Instruction::has_ret_cont() const {
    return false;
}

bool Instruction::always_need_id_gen( CppEmitter *cpp_emitter ) const {
    return false;
}

} // namespace Hpipe

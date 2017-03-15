#include "InstructionTestContiguous.h"
#include "InstructionMultiCond.h"
#include "InstructionNextChar.h"
#include "InstructionRewind.h"
#include "InstructionMark.h"
#include "InstructionNone.h"
#include "InstructionSave.h"
#include "InstructionOK.h"
#include "InstructionKO.h"
#include "CppEmitter.h"
#include <algorithm>

namespace Hpipe {

InstructionNextChar::InstructionNextChar( const Context &cx, bool beg ) : Instruction( cx ), beg( beg ) {
}

void InstructionNextChar::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "+1";
    if ( beg )
        os << "(B)";
}

Transition *InstructionNextChar::train( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous ) {
    if ( ++s - beg >= inp.size() )
        return next.size() > 1 ? &next[ 1 ] : 0;
    return &next[ 0 ];
}

Instruction *InstructionNextChar::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    if ( keep_ind.size() == 1 and keep_ind[ 0 ] == 1 )
        return inst_pool << new InstructionNone( ncx );
    return inst_pool << new InstructionNextChar( ncx, beg );
}

void InstructionNextChar::boyer_moore_opt( PtrPool<Instruction> &inst_pool, Instruction **init ) {
    if ( op_id == Instruction::cur_op_id )
        return;
    op_id = Instruction::cur_op_id;
    
    bool avoid_inlined_next = false;

    Vec<std::pair<Vec<Cond>,Instruction *>> front; // seq of conds to instructions with code
    front.emplace_back( Cond{ 0, 255 }, next[ 0 ].inst );
    while ( true ) {
        // use conds, keep +1 and skip the other kinds of instructions
        bool has_code = false;
        for( unsigned num_in_front = 0; num_in_front < front.size(); ++num_in_front ) {
            Instruction *&inst = front[ num_in_front ].second;
            if ( not avoid_inlined_next )
                inst->op_id = Instruction::cur_op_id;

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

        // use next_chars
        for( unsigned num_in_front = 0; num_in_front < front.size(); ++num_in_front ) {
            Instruction *&inst = front[ num_in_front ].second;
            //if ( not avoid_inlined_next )
            //    inst->op_id = Instruction::cur_op_id;
            inst = inst->next[ 0 ].inst;
            front[ num_in_front ].first.emplace_back( 0, 255 );
        }

        if ( front.size() and front[ 0 ].first.size() >= 50 ) {
            std::cerr << "Limit of boyer-moore optimization (50 chars) exceeded." << std::endl;
            return;
        }
    }

    //  if only 1 cond -> nothing to optimize
    if ( front.size() == 0 or front[ 0 ].first.size() <= 1 )
        return;

    // make and place the new instruction
    InstructionTestContiguous *test = inst_pool << new InstructionTestContiguous( cx, beg, front[ 0 ].first.size() );
    Instruction *ok = make_boyer_moore_rec( inst_pool, front, this, front[ 0 ].first.size() );
    test->next << ok;
    ok->prev << test;

    insert_before_this( test, *init );

    // follow
    for( const auto &p : front )
        p.second->boyer_moore_opt( inst_pool, init );
}

Instruction *InstructionNextChar::make_boyer_moore_rec( PtrPool<Instruction> &inst_pool, const Vec<std::pair<Vec<Cond>, Instruction *> > &front, InstructionNextChar *next_char, int orig_front_size ) {
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

        Instruction *next = make_boyer_moore_rec( inst_pool, new_front, next_char, orig_front_size );
        res->next << next;
        next->prev << res;
    }
    // PRINT( conds );

    return res;

}

bool InstructionNextChar::always_need_id_gen( CppEmitter *cpp_emitter ) const {
    return cpp_emitter->interruptible();
}

void InstructionNextChar::get_code_repr( std::ostream &os ) {
    os << "NEXT_CHAR " << beg << " " << bool( mark );
}

void InstructionNextChar::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    if ( cpp_emitter->interruptible() ) {
        unsigned cont_label = ++cpp_emitter->nb_cont_label;
        ss << "if ( data " << ( beg ? ">" : ">=" ) << " end_m1 ) goto c_" << cont_label << ";"; // this one is good but it won't the case for the next
        if ( ! beg )
            ss << "++data;";

        // c_... (code when there is no data left in the buffer)
        es.rm_beg( 2 ) << "c_" << cont_label << ":" << ( cpp_emitter->trace_labels ? " std::cout << \"c_" + to_string( cont_label ) + " \" << __LINE__ << std::endl;" : "" );
        if ( need_buf_next() )
            es << "while ( buf->next ) {" << ( need_buf_next() > 1 ? " HPIPE_BUFFER::inc_ref( buf, " + to_string( need_buf_next() - 1 ) + " );" : "" ) << " buf = buf->next; if ( buf->used ) { data = buf->data; end_m1 = buf->data + buf->used - 1; goto l_" << next[ 0 ].inst->get_id_gen( cpp_emitter ) << "; } }";
        else
            es << "while ( buf->next ) { HPIPE_BUFFER *old = buf; buf = buf->next; HPIPE_BUFFER::dec_ref( old ); if ( buf->used ) { data = buf->data; end_m1 = buf->data + buf->used - 1; goto l_" << next[ 0 ].inst->get_id_gen( cpp_emitter ) << "; } }";

        if ( next.size() >= 2 )
            es << "if ( last_buf ) goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";

        if ( need_buf_next() ) {
            es << "sipe_data->inp_cont = &&e_" << cont_label << ";";
            es << "HPIPE_BUFFER::inc_ref( buf, " << need_buf_next() << " );";
            es << "return RET_CONT;";

            // e_... (come back code)
            es.rm_beg( 2 ) << "e_" << cont_label << ":" << ( cpp_emitter->trace_labels ? " std::cout << \"e_" + to_string( cont_label ) + " \" << __LINE__ << std::endl;" : "" );
            es << "sipe_data->pending_buf->next = buf;";
            es << "sipe_data->pending_buf = buf;";
            es << "if ( data > end_m1 ) goto c_" << cont_label << ";";
            es << "goto l_" << next[ 0 ].inst->get_id_gen( cpp_emitter ) << ";";
        } else {
            es << "sipe_data->inp_cont = &&e_" << cont_label << ";";
            es << "return RET_CONT;";

            // e_... (come back code)
            es.rm_beg( 2 ) << "e_" << cont_label << ":";
            es << "if ( data > end_m1 ) goto c_" << cont_label << ";";
            es << "goto l_" << next[ 0 ].inst->get_id_gen( cpp_emitter ) << ";";
        }
    } else if ( cpp_emitter->buffer_type == CppEmitter::C_STR ) {
        if ( next.size() >= 2 )
            ss << "if ( *" << ( beg ? "data" : "( ++data )" ) << " == " << cpp_emitter->end_char << " ) goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
        else if ( ! beg )
            ss << "++data;";
    } else { // not interruptible
        if ( next.size() >= 2 )
            ss << "if ( data " << ( beg ? ">" : ">=" ) << " end_m1 ) goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
        if ( ! beg )
            ss << "++data;";
    }

    write_trans( ss, cpp_emitter );
}

} // namespace Hpipe

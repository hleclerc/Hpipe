#include "InstructionCond.h"
#include "InstructionNone.h"

namespace Hpipe {

InstructionCond::InstructionCond( const Context &cx, const Cond &cond, int off_data, const Cond &not_in ) : Instruction( cx ), cond( cond ), not_in( not_in ), off_data( off_data ) {
}

void InstructionCond::write_dot( std::ostream &os ) const {
    os << cond; // << " not_in " << not_in;
    if ( off_data )
        os << " O(" << off_data << ")";
}

Instruction *InstructionCond::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    if ( ncx.pos.size() == 1 )
        return inst_pool << new InstructionNone( ncx );
    return inst_pool << new InstructionCond( ncx, cond, off_data, not_in );
}

Transition *InstructionCond::train( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous ) {
    return s + off_data < inp.size() ? &next[ not cond[ inp[ s + off_data ] ] ] : 0;
}

void InstructionCond::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    if ( next[ 1 ].inst->num_ordering == num_ordering + 1 ) {
        ss << "if ( " << cond.ok_cpp( "data[ " + to_string( off_data ) + " ]", &not_in ) << " ) goto l_" << next[ 0 ].inst->get_id_gen( cpp_emitter ) << ";";
    } else {
        ss << "if ( " << cond.ko_cpp( "data[ " + to_string( off_data ) + " ]", &not_in ) << " ) goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
        write_trans( ss, cpp_emitter );
    }

    //    // if (the most frequent) transition is for the next inst, or (the most frequent) transition goes to the past
    //    if ( transitions[ 0 ].inst->num_ordering == num_ordering + 1 ) {
    //        ss << "if ( " << cond.ko_cpp( "data[" + to_string( off_data ) + "]", &covered ) << " ) goto l_" << transitions[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
    //    } else if ( transitions[ 0 ].inst->num_ordering <= num_ordering or transitions[ 1 ].inst->num_ordering <= num_ordering ) {
    //        ss << "if ( " << cond.ok_cpp( "data[" + to_string( off_data ) + "]", &covered ) << " ) goto l_" << transitions[ 0 ].inst->get_id_gen( cpp_emitter ) << ";";
    //        if ( transitions[ 1 ].inst->num_ordering != num_ordering + 1 )
    //            ss << "goto l_" << transitions[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
    //    } else {
    //        ss << "if ( " << cond.ko_cpp( "data[" + to_string( off_data ) + "]", &covered ) << " ) goto l_" << transitions[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
    //        ss << "goto l_" << transitions[ 0 ].inst->get_id_gen( cpp_emitter ) << ";";
    //    }
}

Instruction *InstructionCond::make_cond( const BranchSet::Node *node, PtrPool<Instruction> &inst_pool, const Cond &not_in, bool in_a_cycle, InstructionMark *mark, int off_data ) {
    if ( node->ok ) {
        if ( node->use_equ ) {
            Cond cond( node->beg, node->beg );
            if ( node->use_neg ) {
                Instruction *res = inst_pool << new InstructionCond( {}, cond, off_data, not_in );
                Instruction *ok = make_cond( node->ok.ptr(), inst_pool, not_in | ~ cond, in_a_cycle, mark, off_data );
                Instruction *ko = make_cond( node->ko.ptr(), inst_pool, not_in |   cond, in_a_cycle, mark, off_data );
                res->next << Transition( ok, {}, node->ok->freq ); ok->prev << res;
                res->next << Transition( ko, {}, node->ko->freq ); ko->prev << res;
                res->in_a_cycle = in_a_cycle;
                res->mark = mark;
                return res;
            }
            Instruction *res = inst_pool << new InstructionCond( {}, ~ cond, off_data, not_in );
            Instruction *ko = make_cond( node->ko.ptr(), inst_pool, not_in |   cond, in_a_cycle, mark, off_data );
            Instruction *ok = make_cond( node->ok.ptr(), inst_pool, not_in | ~ cond, in_a_cycle, mark, off_data );
            res->next << Transition( ko, {}, node->ko->freq ); ko->prev << res;
            res->next << Transition( ok, {}, node->ok->freq ); ok->prev << res;
            res->in_a_cycle = in_a_cycle;
            res->mark = mark;
            return res;
        }
        Cond cond( node->beg, 255 );
        if ( node->use_neg ) {
            Instruction *res = inst_pool << new InstructionCond( {}, ~ cond, off_data, not_in );
            Instruction *ok = make_cond( node->ok.ptr(), inst_pool, not_in |   cond, in_a_cycle, mark, off_data );
            Instruction *ko = make_cond( node->ko.ptr(), inst_pool, not_in | ~ cond, in_a_cycle, mark, off_data );
            res->next << Transition( ok, {}, node->ok->freq ); ok->prev << res;
            res->next << Transition( ko, {}, node->ko->freq ); ko->prev << res;
            res->in_a_cycle = in_a_cycle;
            res->mark = mark;
            return res;
        }
        Instruction *res = inst_pool << new InstructionCond( {}, cond, off_data, not_in );
        Instruction *ko = make_cond( node->ko.ptr(), inst_pool, not_in | ~ cond, in_a_cycle, mark, off_data );
        Instruction *ok = make_cond( node->ok.ptr(), inst_pool, not_in |   cond, in_a_cycle, mark, off_data );
        res->next << Transition( ko, {}, node->ko->freq ); ko->prev << res;
        res->next << Transition( ok, {}, node->ok->freq ); ok->prev << res;
        res->in_a_cycle = in_a_cycle;
        res->mark = mark;
        return res;
    }
    return node->inst;
}

void InstructionCond::optimize_conditions( PtrPool<Instruction> &inst_pool ) {
    return;


//    if ( op_id == Instruction::cur_op_id )
//        return;
//    op_id = Instruction::cur_op_id;

//    std::map<Instruction *,Cond> leaves;
//    find_cond_leaves( leaves, { 0, 255 }, this );

//    // char => instruction
//    Instruction *inst[ 256 ];
//    for( auto &p : leaves )
//        for( unsigned i = 0; i < 256; ++i )
//            if ( p.second[ i ] )
//                inst[ i ] = p.first;
//    for( unsigned i = 0; i < 256; ++i )
//        if ( not_in[ i ] )
//            inst[ i ] = 0;

//    // input for BranchSet
//    Vec<BranchSet::Range> ranges;
//    double cum_freq = 0;
//    int o = 0;
//    while ( o < 256 and not inst[ o ] )
//        ++o;
//    for( int i = o + 1; i < 256; ++i ) {
//        if ( inst[ i ] and inst[ i ] != inst[ o ] ) {
//            ranges.emplace_back( BranchSet::Range{ o, i, inst[ o ], nullptr, cum_freq } );
//            cum_freq = 0;
//            o = i;
//        }
//        cum_freq += freq.size() ? freq[ i ] : 0;
//    }
//    if ( o != 256 )
//        ranges.emplace_back( BranchSet::Range{ o, 256, inst[ o ], nullptr, cum_freq } );

//    // to avoid freq 0
//    std::map<Instruction *,double> nb_ranges;
//    for( BranchSet::Range &range : ranges )
//        ++nb_ranges[ range.inst ];
//    for( BranchSet::Range &range : ranges )
//        range.freq += 1e-3 / nb_ranges[ range.inst ];

//    //
//    BranchSet best_bs( ranges );
//    repl_in_preds( make_cond( best_bs.root.ptr(), inst_pool, {}, in_a_cycle, mark, off_data ) );

//    // following conditions
//    for( auto &p : leaves )
//        p.first->optimize_conditions( inst_pool );
}

void InstructionCond::find_cond_leaves( std::map<Instruction *,Cond> &leaves, const Cond &in, Instruction *orig ) {
    next[ 0 ].inst->find_cond_leaves( leaves, in &  cond, this );
    next[ 1 ].inst->find_cond_leaves( leaves, in & ~cond, this );
}

bool InstructionCond::same_code( const Instruction *_that ) const {
    const InstructionCond *that = dynamic_cast<const InstructionCond *>( _that );
    return that and cond.ok_cpp( "data", &not_in ) == that->cond.ok_cpp( "data", &that->not_in ) and off_data == that->off_data;
}



} // namespace Hpipe

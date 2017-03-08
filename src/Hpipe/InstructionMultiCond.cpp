#include "InstructionMultiCond.h"
#include "InstructionNone.h"
#include "InstructionCond.h"
#include <algorithm>
#include <sstream>

namespace Hpipe {

InstructionMultiCond::InstructionMultiCond( const Context &cx, const Vec<Cond> &conds, int off_data ) : Instruction( cx ), conds( conds ), off_data( off_data ) {
}

void InstructionMultiCond::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "MC";
    if ( edge_labels )
        for( unsigned i = 0; i < conds.size(); ++i )
            edge_labels->emplace_back( to_string( conds[ i ] ) );
    else
        for( unsigned i = 0; i < conds.size() - 1; ++i )
            os << " " << conds[ i ];
    if ( off_data )
        os << " O(" << off_data << ")";

}

Instruction *InstructionMultiCond::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    Vec<Cond> nconds;
    nconds.reserve( keep_ind.size() );
    for( unsigned ind : keep_ind )
        nconds << conds[ ind ];
    return inst_pool << new InstructionMultiCond( ncx, nconds );
}

Transition *InstructionMultiCond::train( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous ) {
    if ( s + off_data >= inp.size() )
        return 0;
    for( unsigned i = 0; i < conds.size(); ++i )
        if ( conds[ i ][ inp[ s + off_data ] ] )
            return &next[ i ];
    return 0;
}

void InstructionMultiCond::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    //    // jump table
    //    Vec<std::string> jt( Vec<std::string>::Size(), 256 );
    //    for( unsigned i = 0; i < conds.size(); ++i )
    //        for( unsigned j = 0; j < 256; ++j )
    //            if ( conds[ i ][ j ] )
    //                jt[ j ] = "&&l_" + to_string( next[ i ].inst->get_id_gen( cpp_emitter ) );
    //    ss << "{";
    //    ss << "    static void *targets[256] = {";
    //    for( unsigned j = 0; j < 256; ) {
    //        *ss.stream << ss.beg + "        ";
    //        for( unsigned i = 0; i < 8; ++j, ++i )
    //            *ss.stream << jt[ j ] << ", ";
    //        *ss.stream << ss.end;
    //    }
    //    ss << "    };";
    //    ss << "    goto *targets[ data[ " + to_string( off_data ) + " ] ];";
    //    ss << "}";

    //    std::set<Instruction *> insts;
    //    for( const Transition &t : next )
    //        insts.insert( t.inst );
    //    PRINT( insts.size() );

    //    Cond tot_covered;
    //    Vec<std::tuple<Instruction *,Cond,double>> cond_seq;
    //    for( unsigned i = 0; i < conds.size(); ++i ) {
    //        cond_seq.emplace_back( next[ i ].inst, conds[ i ] );
    //        tot_covered |= conds[ i ];
    //    }
    //    std::sort( cond_seq.begin(), cond_seq.end(), []( const auto &a, const auto &b ) {
    //        return a.first->freq > b.first->freq;
    //    } );
    //    for( std::pair<Instruction *,Cond> &c : cond_seq )
    //        PRINT( c.first->freq );

    //    Cond covered;
    //    for( unsigned i = 0; i < conds.size(); ++i ) {
    //        if ( next[ i ].inst->num_ordering == num_ordering + 1 )
    //            continue;
    //        covered |= conds[ i ];
    //    }

    // simple succession of tests
    Cond covered;
    for( unsigned i = 0; i < conds.size(); ++i ) {
        if ( next[ i ].inst->num_ordering == num_ordering + 1 )
            continue;

        std::string tst = conds[ i ].ok_cpp( "data[ " + to_string( off_data ) + " ]", &covered );
        if ( tst == "true" ) {
            ss << "goto l_" << next[ i ].inst->get_id_gen( cpp_emitter ) << ";";
            break;
        }
        ss << "if ( " << tst << " ) goto l_" << next[ i ].inst->get_id_gen( cpp_emitter ) << ";";
        covered |= conds[ i ];
    }
}

void InstructionMultiCond::optimize_conditions( PtrPool<Instruction> &inst_pool ) {
    if ( op_id == Instruction::cur_op_id )
        return;
    op_id = Instruction::cur_op_id;

    // char => instruction
    Instruction *inst[ 256 ];
    for( unsigned i = 0; i < 256; ++i )
        inst[ i ] = 0;
    for( unsigned n = 0; n < conds.size(); ++n )
        for( unsigned i = 0; i < 256; ++i )
            if ( conds[ n ][ i ] )
                inst[ i ] = next[ n ].inst;

    // input for BranchSet
    Vec<BranchSet::Range> ranges;
    double cum_freq = 0;
    int o = 0;
    while ( o < 256 and not inst[ o ] )
        ++o;
    for( int i = o + 1; i < 256; ++i ) {
        if ( inst[ i ] and inst[ i ] != inst[ o ] ) {
            ranges.emplace_back( BranchSet::Range{ o, i, inst[ o ], nullptr, cum_freq } );
            cum_freq = 0;
            o = i;
        }
        cum_freq += freq.size() ? freq[ i ] : 0;
    }
    if ( o != 256 )
        ranges.emplace_back( BranchSet::Range{ o, 256, inst[ o ], nullptr, cum_freq } );

    // to avoid freq 0
    std::map<Instruction *,double> nb_ranges;
    for( BranchSet::Range &range : ranges )
        ++nb_ranges[ range.inst ];
    for( BranchSet::Range &range : ranges )
        range.freq += 1e-3 / nb_ranges[ range.inst ];

    // if succession of cond seams to be a better solution, use them
    BranchSet best_bs( ranges );
    repl_in_preds( InstructionCond::make_cond( best_bs.root.ptr(), inst_pool, {}, in_a_cycle, 0, off_data ) );
    for( Transition &t : next )
        t.inst->prev.remove_first_checking( [&]( Transition &p ) { return p.inst == this; } );

    // following conditions
    for( Transition &t : next )
        t.inst->optimize_conditions( inst_pool );
}

void InstructionMultiCond::merge_eq_next( PtrPool<Instruction> &inst_pool ) {
    for( unsigned i = 0; i < next.size(); ++i ) {
        for( unsigned j = i + 1; j < next.size(); ++j ) {
            if ( next[ i ].inst == next[ j ].inst ) {
                next[ j ].inst->prev.remove_first_checking( [&]( const Transition &t ) {
                    return t.inst == this;
                } );
                conds[ i ] |= conds[ j ];
                conds.remove( j );
                next .remove( j );
                --j;
            }
        }
    }
}

bool InstructionMultiCond::can_be_deleted() const {
    return conds.size() == 1;
}

void InstructionMultiCond::get_code_repr( std::ostream &os ) {
    os << "MC " << off_data << " " << conds.size();
    for( const Cond &cond: conds ) {
        std::ostringstream ss;
        ss << cond;
        os << " " << ss.str().size() << " " << ss.str();
    }
}

} // namespace Hpipe

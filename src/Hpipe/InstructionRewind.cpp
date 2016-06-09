#include "InstructionWithCode.h"
#include "InstructionRewind.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionRewind::InstructionRewind( const Context &cx ) : Instruction( cx ), exec( 0 ) {
}

void InstructionRewind::write_dot( std::ostream &os ) const {
    if ( use_exec() ) {
        os << "RW_" << get_display_id();
    } else if ( use_code_seq() ) {
        os << "RS";
        for( InstructionWithCode *inst : code_seq )
            inst->write_dot( os << "\n" );
    } else
        os << "FM";
}

Instruction *InstructionRewind::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    return inst_pool << new InstructionRewind( ncx );
}

void InstructionRewind::apply_rec( std::function<void(Instruction *)> f, bool subgraphs ) {
    if ( op_id == Instruction::cur_op_id )
        return;
    op_id = Instruction::cur_op_id;

    f( this );

    for( Transition &t : next )
        t.inst->apply_rec( f, subgraphs );

    if ( subgraphs and use_exec() )
        exec->apply_rec( f, subgraphs );
}

void InstructionRewind::get_unused_rec( Vec<Instruction *> &to_remove, Instruction *&init ) {
    if ( op_id == Instruction::cur_op_id )
        return;

    Instruction::get_unused_rec( to_remove, init );

    if ( use_exec() )
        exec->get_unused_rec( to_remove, exec );
}

void InstructionRewind::apply_rec_rewind_l( std::function<void(Instruction *, unsigned)> f, unsigned rewind_level ) {
    if ( op_id == Instruction::cur_op_id )
        return;
    op_id = Instruction::cur_op_id;

    f( this, rewind_level );

    for( Transition &t : next )
        t.inst->apply_rec_rewind_l( f, rewind_level );

    if ( use_exec() )
        exec->apply_rec_rewind_l( f, rewind_level + 1 );
}

bool InstructionRewind::use_exec() const {
    return exec and has_code_in_a_cycle;
}

bool InstructionRewind::use_code_seq() const {
    return not has_code_in_a_cycle and not mark;
}

bool InstructionRewind::with_code() const {
    return use_exec() or ( not has_code_in_a_cycle and code_seq.size() );
}

void InstructionRewind::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    if ( use_exec() ) {
        // ss << "// beg rewind";
        if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER ) {
            ss << "data   = " << ( cpp_emitter->rewind_rec_level or not cpp_emitter->interruptible() ? "rw_ptr[ " + to_string( cpp_emitter->rewind_rec_level - cpp_emitter->interruptible() ) + " ]" : "sipe_data->rw_ptr" ) << ";";
            ss << "buf    = " << ( cpp_emitter->rewind_rec_level or not cpp_emitter->interruptible() ? "rw_buf[ " + to_string( cpp_emitter->rewind_rec_level - cpp_emitter->interruptible() ) + " ]" : "sipe_data->rw_buf" ) << ";";
            ss << "end_m1 = buf->data - 1 + buf->used;";
        } else
            ss << "data = " << ( cpp_emitter->rewind_rec_level or not cpp_emitter->interruptible() ? "rw_ptr[ " + to_string( cpp_emitter->rewind_rec_level - cpp_emitter->interruptible() ) + " ]" : "sipe_data->rw_ptr" ) << ";";

        Instruction *old_inst_to_go_if_ok = cpp_emitter->inst_to_go_if_ok;
        cpp_emitter->inst_to_go_if_ok = next[ 0 ].inst;
        ++cpp_emitter->rewind_rec_level;

        cpp_emitter->write_parse_body( ss, exec );
        // ss << "// end rewind";

        --cpp_emitter->rewind_rec_level;
        cpp_emitter->inst_to_go_if_ok = old_inst_to_go_if_ok;
    } else if ( use_code_seq() ) {
        for( InstructionWithCode *inst : code_seq )
            inst->write_cpp_code_seq( ss, es, cpp_emitter );
    } else { // free mark
        if ( mark and cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER and cpp_emitter->rewind_rec_level == 0 )
            ss << "sipe_data->rw_buf->dec_ref_upto( buf );";
    }

    write_trans( ss, cpp_emitter );
}

bool InstructionRewind::merge_predecessors( Instruction **init ) {
    return Instruction::merge_predecessors( init ) or (
                use_exec() and
                exec->find_rec( [ this ]( Instruction *inst ) { return inst->merge_predecessors(); } ) );
}

bool InstructionRewind::same_code( const Instruction *_that ) const {
    const InstructionRewind *that = dynamic_cast<const InstructionRewind *>( _that );
    if ( that == 0 or bool( mark ) != bool( that->mark ) or has_code_in_a_cycle != that->has_code_in_a_cycle or use_exec() != that->use_exec() )
        return false;
    // case 1: exec
    if ( use_exec() ) {
        // TODO
        return false;
    }
    // case 2: code_seq
    if ( code_seq.size() != that->code_seq.size() )
        return false;
    for( unsigned i = 0; i < code_seq.size(); ++i)
        if ( not code_seq[ i ]->same_code( that->code_seq[ i ] ) or code_seq[ i ]->save != that->code_seq[ i ]->save )
            return false;
    return true;
}

void InstructionRewind::optimize_conditions( PtrPool<Instruction> &inst_pool ) {
    if ( op_id == Instruction::cur_op_id )
        return;
    op_id = Instruction::cur_op_id;

    for( Transition &t : next )
        t.inst->optimize_conditions( inst_pool );

    if ( use_exec() )
        exec->optimize_conditions( inst_pool );
}

Transition *InstructionRewind::train( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous ) {
    if ( use_exec() ) {
        s = m;
        std::string::size_type nm;
        exec->train_rec( s, nm, inp, freq, use_contiguous );
    }
    return &next[ 0 ];
}

void InstructionRewind::reg_var( std::function<void(std::string, std::string)> f ) {
    if ( not use_exec() )
        for( InstructionWithCode *inst : code_seq )
            inst->reg_var( f );
}

void InstructionRewind::write_dot_add( std::ostream &os, bool disp_inst_pred, bool disp_trans_freq, bool disp_rc_item ) const {
    if ( mark )
        os << "  node_" << this << " -> node_" << mark << " [color=green];\n";

    if ( use_exec() ) {
        // os << "  node_" << this << " -> node_" << rewind_exec << " [style=dashed,color=red,rank=same];\n";
        os << "subgraph cluster_" << this << " {\n";
        os << "  label = \"RW_" << get_display_id() << "\";\n";
        os << "  color = gray;\n";
        exec->write_dot_rec( os, disp_inst_pred, disp_trans_freq, disp_rc_item );
        os << "}\n";
    }
}


} // namespace Hpipe

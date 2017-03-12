#include "InstructionWithCode.h"
#include "InstructionRewind.h"
#include "InstructionSave.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionRewind::InstructionRewind( const Context &cx ) : Instruction( cx ) {
}

void InstructionRewind::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    if ( need_rw ) {
        os << "RW_" << get_display_id();
    } else if ( code_seq.size() ) {
        os << "RW_SEQ" << get_display_id();
        for( InstructionWithCode *inst : code_seq ) {
            if ( inst->save )
                inst->write_dot( os << " S" << inst->save->num_save << "\n" );
            else
                inst->write_dot( os << "\n" );
        }
    } else
        os << "FM_" << get_display_id();
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

    //    if ( subgraphs and need_rw )
    //        exec->apply_rec( f, subgraphs );
}

void InstructionRewind::get_unused_rec( Vec<Instruction *> &to_remove, Instruction *&init ) {
    if ( op_id == Instruction::cur_op_id )
        return;

    Instruction::get_unused_rec( to_remove, init );

    //    if ( need_rw )
    //        exec->get_unused_rec( to_remove, exec );
}

void InstructionRewind::apply_rec_rewind_l( std::function<void(Instruction *, unsigned)> f, unsigned rewind_level ) {
    if ( op_id == Instruction::cur_op_id )
        return;
    op_id = Instruction::cur_op_id;

    f( this, rewind_level );

    for( Transition &t : next )
        t.inst->apply_rec_rewind_l( f, rewind_level );

    //    if ( need_rw )
    //        exec->apply_rec_rewind_l( f, rewind_level + 1 );
}

bool InstructionRewind::with_code() const {
    return need_rw or code_seq.size();
}

void InstructionRewind::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER ) {
        ss << "data   = sipe_data->rw_ptr;";
        ss << "buf    = sipe_data->rw_buf;";
        ss << "end_m1 = buf->data - 1 + buf->used;";
    } else
        ss << "data = rw_ptr[ 0 ];";

    for( InstructionWithCode *inst : code_seq )
        inst->write_cpp_code_seq( ss, es, cpp_emitter );

    //    if ( need_rw ) {
    //        // ss << "// beg rewind";
    //        if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER ) {
    //            ss << "data   = " << ( cpp_emitter->rewind_rec_level or not cpp_emitter->interruptible() ? "rw_ptr[ " + to_string( cpp_emitter->rewind_rec_level - cpp_emitter->interruptible() ) + " ]" : "sipe_data->rw_ptr" ) << ";";
    //            ss << "buf    = " << ( cpp_emitter->rewind_rec_level or not cpp_emitter->interruptible() ? "rw_buf[ " + to_string( cpp_emitter->rewind_rec_level - cpp_emitter->interruptible() ) + " ]" : "sipe_data->rw_buf" ) << ";";
    //            ss << "end_m1 = buf->data - 1 + buf->used;";
    //        } else
    //            ss << "data = " << ( cpp_emitter->rewind_rec_level or not cpp_emitter->interruptible() ? "rw_ptr[ " + to_string( cpp_emitter->rewind_rec_level - cpp_emitter->interruptible() ) + " ]" : "sipe_data->rw_ptr" ) << ";";

    //        Instruction *old_inst_to_go_if_ok = cpp_emitter->inst_to_go_if_ok;
    //        cpp_emitter->inst_to_go_if_ok = next[ 0 ].inst;
    //        ++cpp_emitter->rewind_rec_level;

    //        cpp_emitter->write_parse_body( ss, exec );

    //        --cpp_emitter->rewind_rec_level;
    //        cpp_emitter->inst_to_go_if_ok = old_inst_to_go_if_ok;
    //    } else {
    //        for( InstructionWithCode *inst : code_seq )
    //            inst->write_cpp_code_seq( ss, es, cpp_emitter );
    //        // free mark
    //        //        if ( mark && cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER && cpp_emitter->rewind_rec_level == 0 )
    //        //            ss << "sipe_data->rw_buf->dec_ref_upto( buf );";

    //        write_trans( ss, cpp_emitter );
    //    }
}

void InstructionRewind::optimize_conditions( PtrPool<Instruction> &inst_pool ) {
    if ( op_id == Instruction::cur_op_id )
        return;
    op_id = Instruction::cur_op_id;

    for( Transition &t : next )
        t.inst->optimize_conditions( inst_pool );

    //    if ( need_rw )
    //        exec->optimize_conditions( inst_pool );
}

Transition *InstructionRewind::train( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous ) {
    //    if ( need_rw ) {
    //        s = m;
    //        std::string::size_type nm;
    //        exec->train_rec( s, nm, inp, freq, use_contiguous );
    //    }
    return &next[ 0 ];
}

void InstructionRewind::reg_var( std::function<void(std::string, std::string)> f ) {
    if ( not need_rw )
        for( InstructionWithCode *inst : code_seq )
            inst->reg_var( f );
}

void InstructionRewind::get_code_repr( std::ostream &os ) {
    //    if ( need_rw ) {
    //        os << "RW_EXEC";
    //        // get instructions in the graph
    //        Vec<Instruction *> insts;
    //        exec->apply( [&]( Instruction *inst ) {
    //            inst->op_mp = insts.size();
    //            insts << inst;
    //        } );
    //        os << " " << insts.size();
    //        // for each inst, write code repr + links
    //        for( Instruction *inst : insts ) {
    //            std::string ss = inst->code_repr();
    //            os << " " << ss.size() << " " << ss << " " << inst->next.size();
    //            for( Transition &trans : inst->next )
    //                os << " " << trans.inst->op_mp;
    //        }
    //    } else {
    os << "RW_SEQ " << code_seq.size();
    for( InstructionWithCode *inst : code_seq ) {
        if ( inst->save )
            os << " S" << inst->save->num_save;
        else
            os << " N";

        std::string ss = inst->code_repr();
        os << " " << ss.size() << " " << ss;
    }
    //        if ( mark )
    //            os << "FM";
    //    }
}

void InstructionRewind::write_dot_add( std::ostream &os, bool disp_inst_pred, bool disp_trans_freq, bool disp_rc_item ) const {
    //    if ( mark )
    //        os << "  node_" << this << " -> node_" << mark << " [color=green];\n";

    //    if ( exec ) { // need_rw
    //        // os << "  node_" << this << " -> node_" << rewind_exec << " [style=dashed,color=red,rank=same];\n";
    //        os << "subgraph cluster_" << this << " {\n";
    //        os << "  label = \"RW_" << get_display_id() << ( need_rw ? " need_rw" : "" ) << " " << code_seq.size() << "\";\n";
    //        os << "  color = gray;\n";
    //        exec->write_dot_rec( os, disp_inst_pred, disp_trans_freq, disp_rc_item );
    //        os << "}\n";
    //    }
}


} // namespace Hpipe

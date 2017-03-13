#include "InstructionWithCode.h"
#include "InstructionRewind.h"
#include "InstructionSave.h"
#include "InstructionMark.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionRewind::InstructionRewind( const Context &cx ) : Instruction( cx ), exec( 0 ) {
    cx.mark->rewinds << this;
}

void InstructionRewind::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "RW" << get_display_id();

    if ( code_seq_beg.size() )
        for( const CodeSeqItem &item : code_seq_beg )
            item.code->write_dot( os << "\nB" << item.offset );

    if ( code_seq_end.size() )
        for( const CodeSeqItem &item : code_seq_end )
            item.code->write_dot( os << "\nE" << item.offset );

    //    if ( need_rw ) {
    //        os << "RW_" << get_display_id();
    //    } else
    //        os << "FM_" << get_display_id();
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
    return need_rw || code_seq_beg.size() || code_seq_end.size();
}

void InstructionRewind::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    ss << "// RW";
    if ( need_rw ) {
        if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER ) {
            ss << "data   = sipe_data->rw_ptr;";
            ss << "buf    = sipe_data->rw_buf;";
            ss << "end_m1 = buf->data + buf->used - 1;";
        } else
            ss << "data = rw_ptr;";

        // code_seq_beg
        unsigned old_offset = 0;
        for( const CodeSeqItem &item : code_seq_beg ) {
            // need to skip some bytes ?
            if ( item.offset >= 0 && item.offset != old_offset ) {
                if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER )
                    ss << "if ( HPIPE_BUFFER::skip( buf, data, " << item.offset - old_offset << " ) ) end_m1 = buf->data + buf->used - 1;";
                else
                    ss << "data += " << item.offset - old_offset << ";";
                old_offset = item.offset;
            }
            item.code->write_cpp_code_seq( ss, es, cpp_emitter );
        }

        // need to skip some bytes ?
        if ( use_of_ncx == USE_NCX_END ) {
            HPIPE_TODO;
        } else {
            if ( offset_for_ncx != old_offset ) {
                if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER )
                    ss << "if ( HPIPE_BUFFER::skip( buf, data, " << offset_for_ncx - old_offset << " ) ) end_m1 = buf->data + buf->used - 1;";
                else
                    ss << "data += " << offset_for_ncx - old_offset << ";";
            }
        }
    } else {
        // do code_seq_beg
        unsigned old_offset = 0;
        for( const CodeSeqItem &item : code_seq_beg ) {
            // need to skip some bytes ?
            if ( item.offset >= 0 && item.offset != old_offset ) {
                if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER )
                    ss << "HPIPE_BUFFER::skip( sipe_data->rw_buf, sipe_data->rw_ptr, " << item.offset - old_offset << " );";
                else
                    ss << "rw_ptr += " << item.offset - old_offset << ";";
                old_offset = item.offset;
            }

            // do the code
            if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER )
                item.code->write_cpp_code_seq( ss, es, cpp_emitter, "sipe_data->rw_ptr", "sipe_data->rw_buf" );
            else
                item.code->write_cpp_code_seq( ss, es, cpp_emitter, "rw_ptr", "rw_buf" );
        }

        // do code_seq_end
        if ( code_seq_end.size() ) {
            // need to skip some bytes ?
            int off = -1;
            for( const CodeSeqItem &item : code_seq_end )
                off = std::max( off, item.offset );

            // if no positionnal code, simply write the code
            if ( off < 0 ) {
                for( const CodeSeqItem &item : code_seq_end )
                    item.code->write_cpp_code_seq( ss, es, cpp_emitter );
            } else {
                // else, jump to position for the first code to be executed
                if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER )
                    ss << "HPIPE_BUFFER::skip( sipe_data->rw_buf, sipe_data->rw_ptr, HPIPE_BUFFER::size_between( sipe_data->rw_buf, sipe_data->rw_ptr, buf, data ) - " << off << " );";
                else
                    ss << "rw_ptr = data - " << off << ";";

                // do the code
                for( unsigned num_item = code_seq_end.size(); num_item--; ) {
                    const CodeSeqItem &item = code_seq_end[ num_item ];

                    // need to skip some bytes ?
                    if ( item.offset >= 0 && item.offset != off ) {
                        if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER )
                            ss << "HPIPE_BUFFER::skip( sipe_data->rw_buf, sipe_data->rw_ptr, " << off - item.offset << " );";
                        else
                            ss << "rw_ptr += " << off - item.offset << ";";
                        off = item.offset;
                    }

                    // do the code
                    if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER )
                        item.code->write_cpp_code_seq( ss, es, cpp_emitter, "sipe_data->rw_ptr", "sipe_data->rw_buf" );
                    else
                        item.code->write_cpp_code_seq( ss, es, cpp_emitter, "rw_ptr", "rw_buf" );
                }
            }
        }

        if ( mark && cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER )
            ss << "sipe_data->rw_buf->dec_ref_upto( buf );";
    }

    write_trans( ss, cpp_emitter );
}

Transition *InstructionRewind::train( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous ) {
    return &next[ 0 ];
}

void InstructionRewind::reg_var( std::function<void(std::string, std::string)> f, CppEmitter *cpp_emitter ) {
    if ( not need_rw ) {
        for( CodeSeqItem &item : code_seq_beg )
            item.code->reg_var( f, cpp_emitter );
        for( CodeSeqItem &item : code_seq_end )
            item.code->reg_var( f, cpp_emitter );
    }
}

void InstructionRewind::get_code_repr( std::ostream &os ) {
    if ( need_rw ) {
        os << "RW " << code_seq_beg.size() << " " << offset_for_ncx << " " << use_of_ncx;
        for( CodeSeqItem &item : code_seq_beg ) {
            std::string ss = item.code->code_repr();
            os << " " << item.offset << " " << ss.size() << " " << ss;
        }
    } else {
        os << "FM " << code_seq_beg.size() << " " << code_seq_end.size();
        for( CodeSeqItem &item : code_seq_beg ) {
            std::string ss = item.code->code_repr();
            os << " " << item.offset << " " << ss.size() << " " << ss;
        }
        for( CodeSeqItem &item : code_seq_end ) {
            std::string ss = item.code->code_repr();
            os << " " << item.offset << " " << ss.size() << " " << ss;
        }
    }
}

bool InstructionRewind::use_data_in_code_seq() const {
    for( const CodeSeqItem &item : code_seq_beg )
        if ( item.offset >= 0 )
            return true;
    for( const CodeSeqItem &item : code_seq_end )
        if ( item.offset >= 0 )
            return true;
    return false;
}

bool InstructionRewind::no_code_at_all() const {
    return code_seq_beg.empty() && code_seq_end.empty();
}

void InstructionRewind::write_dot_add( std::ostream &os, bool disp_inst_pred, bool disp_trans_freq, bool disp_rc_item ) const {
    //    if ( mark )
    //        os << "  node_" << this << " -> node_" << mark << " [color=green];\n";

    if ( need_rw && exec ) {
        // os << "  node_" << this << " -> node_" << rewind_exec << " [style=dashed,color=red,rank=same];\n";
        os << "subgraph cluster_" << this << " {\n";
        os << "  label = \"RW_" << get_display_id() << ( need_rw ? " need_rw" : "" ) << " orx=" << offset_for_ncx << "\";\n";
        os << "  color = gray;\n";
        exec->write_dot_rec( os, disp_inst_pred, disp_trans_freq, disp_rc_item );
        os << "}\n";
    }
}


} // namespace Hpipe

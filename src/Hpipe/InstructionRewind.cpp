#include "InstructionWithCode.h"
#include "InstructionRewind.h"
#include "InstructionSave.h"
#include "InstructionMark.h"
#include "InstructionOK.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionRewind::InstructionRewind( const Context &cx ) : Instruction( cx ), exec( 0 ) {
    cx.mark->rewinds << this;
    offset_ncx = 0;
    need_rw = false;
}

void InstructionRewind::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "RW" << get_display_id();

    if ( code_seq_beg.size() )
        for( const CodeSeqItem &item : code_seq_beg )
            item.code->write_dot( os << "\nB" << item.offset << ":" );

    if ( code_seq_end.size() )
        for( const CodeSeqItem &item : code_seq_end )
            item.code->write_dot( os << "\nE" << item.offset << ":" );

    for( const std::string &str : strs_to_cancel )
        os << "\ncancel " << str;
}

Instruction *InstructionRewind::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    // return inst_pool << new InstructionRewind( ncx );
    return inst_pool << new InstructionOK( ncx );
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
    return need_rw || code_seq_beg.size() || code_seq_end.size() || strs_to_cancel.size();
}

void InstructionRewind::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    ss << "// RW";

    //
    if ( mark )
        for( const std::string &str : strs_to_cancel )
            ss << "HPIPE_DATA.__beg_" << str << "_buf->dec_ref_upto( HPIPE_DATA.rw_buf );";

    // prepare running strings
    std::set<std::string> running_strs;
    for( const auto &p : cx.paths_to_strings )
        running_strs.insert( p.first );

    // need_rw mode => change buf and data (else, only change HPIPE_DATA.rw_buf and HPIPE_DATA.rw_ptr)
    if ( need_rw ) {
        if ( cpp_emitter->buffer_type == CppEmitter::BT_HPIPE_BUFFER ) {
            ss << "data   = HPIPE_DATA.rw_ptr;";
            ss << "buf    = HPIPE_DATA.rw_buf;";
            ss << "end_m1 = buf->data + buf->used - 1;";
        } else
            ss << "data = rw_ptr;";

        // code_seq_beg
        unsigned old_offset = 0;
        for( const CodeSeqItem &item : code_seq_beg ) {
            // need to skip some bytes ?
            if ( item.offset >= 0 && item.offset != old_offset ) {
                if ( cpp_emitter->buffer_type == CppEmitter::BT_HPIPE_BUFFER )
                    ss << "if ( HPIPE_BUFF_T::skip( buf, data, " << item.offset - old_offset << ", " << running_strs.size() << " ) ) end_m1 = buf->data + buf->used - 1;";
                else
                    ss << "data += " << item.offset - old_offset << ";";
                old_offset = item.offset;
            }
            item.code->write_cpp_code_seq( ss, es, cpp_emitter );
            item.code->update_running_strings( running_strs );
        }

        // need to skip some bytes ?
        if ( offset_ncx != old_offset ) {
            if ( cpp_emitter->buffer_type == CppEmitter::BT_HPIPE_BUFFER )
                ss << "if ( HPIPE_BUFF_T::skip( buf, data, " << offset_ncx - old_offset << ", " << running_strs.size() << " ) ) end_m1 = buf->data + buf->used - 1;";
            else
                ss << "data += " << offset_ncx - old_offset << ";";
        }
    } else {
        // do code_seq_beg
        unsigned old_offset = 0;
        for( const CodeSeqItem &item : code_seq_beg ) {
            // need to skip some bytes ?
            if ( item.offset >= 0 && item.offset != old_offset ) {
                if ( cpp_emitter->buffer_type == CppEmitter::BT_HPIPE_BUFFER )
                    ss << "HPIPE_BUFF_T::skip( HPIPE_DATA.rw_buf, HPIPE_DATA.rw_ptr, " << item.offset - old_offset << ", " << running_strs.size() << " );";
                else
                    ss << "rw_ptr += " << item.offset - old_offset << ";";
                old_offset = item.offset;
            }

            // do the code
            if ( cpp_emitter->buffer_type == CppEmitter::BT_HPIPE_BUFFER )
                item.code->write_cpp_code_seq( ss, es, cpp_emitter, "HPIPE_DATA.rw_ptr", "HPIPE_DATA.rw_buf" );
            else
                item.code->write_cpp_code_seq( ss, es, cpp_emitter, "rw_ptr", "rw_buf" );
            item.code->update_running_strings( running_strs );
        }

        // do code_seq_end
        if ( code_seq_end.size() ) {
            // need to skip some bytes ?
            int off = -1;
            for( const CodeSeqItem &item : code_seq_end )
                off = std::max( off, item.offset );

            // if no positionnal code or code on current `data`, simply write the code
            if ( off <= 0 ) {
                for( unsigned num_item = code_seq_end.size(); num_item--; ) {
                    const CodeSeqItem &item = code_seq_end[ num_item ];
                    item.code->write_cpp_code_seq( ss, es, cpp_emitter );
                }
            } else {
                // else, jump to position for the first code to be executed
                if ( cpp_emitter->buffer_type == CppEmitter::BT_HPIPE_BUFFER ) {
                    if ( mark )
                        ss << "HPIPE_BUFF_T::skip( HPIPE_DATA.rw_buf, HPIPE_DATA.rw_ptr, HPIPE_BUFF_T::size_between( HPIPE_DATA.rw_buf, HPIPE_DATA.rw_ptr, buf, data ) - " << off << ", " << running_strs.size() << " );";
                    else {
                        cpp_emitter->loc_vars.push_back_unique( "const HPIPE_CHAR_T *__tmp_data;" );
                        cpp_emitter->loc_vars.push_back_unique( "HPIPE_BUFF_T *__tmp_buf;" );
                        ss << "__tmp_data = HPIPE_DATA.__beg_" << cx.paths_to_strings.begin()->first << "_data; __tmp_buf = HPIPE_DATA.__beg_" << cx.paths_to_strings.begin()->first << "_buf;";
                        ss << "HPIPE_BUFF_T::skip( __tmp_buf, __tmp_data, HPIPE_BUFF_T::size_between( __tmp_buf, __tmp_data, buf, data ) - " << off << ", 1 );";
                    }
                } else
                    ss << "rw_ptr = data - " << off << ";";

                // do the code
                for( unsigned num_item = code_seq_end.size(); num_item--; ) {
                    const CodeSeqItem &item = code_seq_end[ num_item ];

                    // need to skip some bytes ?
                    if ( item.offset >= 0 && item.offset != off ) {
                        if ( cpp_emitter->buffer_type == CppEmitter::BT_HPIPE_BUFFER ) {
                            if ( mark )
                                ss << "HPIPE_BUFF_T::skip( HPIPE_DATA.rw_buf, HPIPE_DATA.rw_ptr, " << off - item.offset << ", " << running_strs.size() << " );";
                            else
                                ss << "HPIPE_BUFF_T::skip( __tmp_buf, __tmp_data, " << off - item.offset << ", 1 );";
                        } else
                            ss << "rw_ptr += " << off - item.offset << ";";
                        off = item.offset;
                    }

                    // do the code
                    if ( cpp_emitter->buffer_type == CppEmitter::BT_HPIPE_BUFFER ) {
                        if ( mark )
                            item.code->write_cpp_code_seq( ss, es, cpp_emitter, "HPIPE_DATA.rw_ptr", "HPIPE_DATA.rw_buf" );
                        else
                            item.code->write_cpp_code_seq( ss, es, cpp_emitter, "__tmp_data", "__tmp_buf" );
                    } else
                        item.code->write_cpp_code_seq( ss, es, cpp_emitter, "rw_ptr", "rw_buf" );
                    item.code->update_running_strings( running_strs );
                }
            }
        }

        if ( mark && cpp_emitter->buffer_type == CppEmitter::BT_HPIPE_BUFFER )
            ss << "HPIPE_DATA.rw_buf->dec_ref_upto( buf, " << running_strs.size() << " );";
    }

    write_trans( ss, cpp_emitter );
}

Transition *InstructionRewind::train( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous ) {
    return &next[ 0 ];
}

void InstructionRewind::get_code_repr( std::ostream &os ) {
    // we use the pointer because we do not want a rewind to go to another mark
    os << mark;
    //
    os << strs_to_cancel.size() << " " << strs_to_cancel << " ";
    //
    if ( need_rw ) {
        os << "RW " << code_seq_beg.size() << " " << offset_ncx;
        for( CodeSeqItem &item : code_seq_beg ) {
            std::string ss = item.code->code_repr();
            os << " " << item.offset << " " << ss.size() << " " << ss;
        }
    } else {
        // if only one non positionnal code
        if ( code_seq_beg.size() == 1 && code_seq_beg[ 0 ].offset == -1 && code_seq_end.size() == 0 )
            return code_seq_beg[ 0 ].code->get_code_repr( os );

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

bool InstructionRewind::need_mark() const {
    if ( need_rw )
        return true;
    // if cpt_use is maintained >= 0 by a running_str, and if no beg, we do not need the mark
    if ( ! mark->running_strs.empty() && code_seq_beg.empty() && strs_to_cancel.empty() )
        return false;
    //
    for( const CodeSeqItem &item : code_seq_beg )
        if ( item.offset >= 0 )
            return true;
    for( const CodeSeqItem &item : code_seq_end )
        if ( item.offset >= 0 )
            return true;
    return false;
}

bool InstructionRewind::no_code_at_all() const {
    return code_seq_beg.empty() && code_seq_end.empty(); // && strs_to_cancel.empty()
}

void InstructionRewind::write_dot_add( std::ostream &os, bool disp_inst_pred, bool disp_trans_freq, bool disp_rc_item ) const {
    //    if ( mark )
    //        os << "  node_" << this << " -> node_" << mark << " [color=green];\n";

    if ( need_rw && exec ) { //
        // os << "  node_" << this << " -> node_" << rewind_exec << " [style=dashed,color=red,rank=same];\n";
        os << "subgraph cluster_" << this << " {\n";
        os << "  label = \"RW_" << get_display_id() << ( need_rw ? " need_rw" : "" ) << " orx=" << offset_ncx << "\";\n";
        os << "  color = gray;\n";
        exec->write_dot_rec( os, disp_inst_pred, disp_trans_freq, disp_rc_item );
        os << "}\n";
    }
}


} // namespace Hpipe

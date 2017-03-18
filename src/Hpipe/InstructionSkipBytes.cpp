#include "InstructionSkipBytes.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionSkipBytes::InstructionSkipBytes( const Context &cx, const std::string &var, bool beg ) : Instruction( cx ), var( var ), beg( beg ) {
}

void InstructionSkipBytes::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "SKIP(" << var << ")";
}

Instruction *InstructionSkipBytes::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    return inst_pool << new InstructionSkipBytes( ncx, var, beg );
}

void InstructionSkipBytes::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    if ( cpp_emitter->buffer_type == CppEmitter::BT_HPIPE_BUFFER ) {
        unsigned cont_label = ++cpp_emitter->nb_cont_label;

        // cpp_emitter->need_loc_var(  );
        std::string v = var;
        if ( beg ) {
            ss << "if ( " << var << " ) {";
            ss.beg += "    ";
            ss << "size_t __l = " << var << " - 1;";
            v = "__l";
        }
        // we can stay in the same buffer
        ss << "if ( data + " << v << " <= end_m1 ) {";
        ss << "    data += " << v << ";";
        ss << "} else {";
        // skip data in current buf
        ss << "    sipe_data->__bytes_to_skip = " << v << " - ( buf->data + buf->used - data );";
        ss << "  t_" << cont_label << ":";
        ss << "    if ( ! buf->next ) {";
        ss << "        if ( last_buf )";
        ss << "            goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
        if ( need_buf_next() ) {
            ss << "        sipe_data->pending_buf = buf;";
            ss << "        HPIPE_BUFFER::inc_ref( buf, " << need_buf_next() << " );";
            ss << "        sipe_data->inp_cont = &&e_" << cont_label << ";";
        } else
            ss << "        sipe_data->inp_cont = &&c_" << cont_label << ";";
        ss << "        return RET_CONT;";
        if ( need_buf_next() ) {
            // e_... (come back code)
            ss << "      e_" << cont_label << ":" << ( cpp_emitter->trace_labels ? " std::cout << \"e_" + to_string( cont_label ) + " \" << __LINE__ << std::endl;" : "" );
            ss << "        sipe_data->pending_buf->next = buf;";
            ss << "        sipe_data->pending_buf = buf;";
            ss << "        goto c_" << cont_label << ";";
        }
        ss << "    }";
        if ( need_buf_next() )
            ss << "    HPIPE_BUFFER::inc_ref( buf, " << need_buf_next() - 1 << " ); buf = buf->next;";
        else
            ss << "    HPIPE_BUFFER *old = buf; buf = buf->next; HPIPE_BUFFER::dec_ref( old );";
        ss << "  c_" << cont_label << ":";
        ss << "    if ( sipe_data->__bytes_to_skip >= buf->used ) {";
        ss << "        sipe_data->__bytes_to_skip -= buf->used;";
        ss << "        goto t_" << cont_label << ";";
        ss << "    }";
        ss << "    data = buf->data + sipe_data->__bytes_to_skip;";
        ss << "    end_m1 = buf->data + buf->used - 1;";
        ss << "}";
        if ( beg ) {
            ss.beg.resize( ss.beg.size() - 4 );
            ss << "}";
        }
    } else if ( cpp_emitter->buffer_type == CppEmitter::BT_C_STR ) {
        if ( beg ) {
            ss << "if ( " << var << " ) {";
            ss.beg += "    ";
        }
        ss << "for( size_t n = ( " << var << ( beg ? " ) - 1" : " )" ) << "; n--; ++data )";
        ss << "    if ( *data == " << cpp_emitter->stop_char << " )";
        ss << "        goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
        ss << "if ( *data == " << cpp_emitter->stop_char << " )";
        ss << "    goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
        if ( beg ) {
            ss.beg.resize( ss.beg.size() - 4 );
            ss << "}";
        }
    } else {
        if ( next.size() > 1 )
            ss << "if ( data + ( " << var << ( beg ? " ) - 1" : " )" ) << " > end_m1 ) goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
        ss << "data += ( " << var << ( beg ? " ) - 1" : " )" ) << ";";
    }
    write_trans( ss, cpp_emitter );
}

Transition *InstructionSkipBytes::train( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous ) {
    return &next[ 0 ]; // HUM. TODO
}

void InstructionSkipBytes::get_code_repr( std::ostream &os ) {
    os << "SKIP_BYTES " << var.size() << " " << var;
}

} // namespace Hpipe

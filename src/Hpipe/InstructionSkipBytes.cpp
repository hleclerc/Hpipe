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
    if ( cpp_emitter->need_buf() ) {
        unsigned cont_label = ++cpp_emitter->nb_cont_label;

        // cpp_emitter->need_loc_var(  );
        std::string v = var;
        if ( cpp_emitter->variables.count( v ) )
            v = "HPIPE_DATA." + v;
        if ( beg ) {
            ss << "if ( " << v << " ) {";
            ss.beg += "    ";
            ss << "size_t __l = " << v << " - 1;";
            v = "__l";
        }
        // we can stay in the same buffer
        ss << "if ( data + " << v << " <= end_m1 ) {";
        ss << "    data += " << v << ";";
        ss << "} else {";
        // skip data in current buf
        cpp_emitter->preliminaries.push_back_unique( "#ifndef HPIPE_SIZE_T\n#define HPIPE_SIZE_T size_t\n#endif // HPIPE_SIZE_T\n" );
        cpp_emitter->add_variable( "__bytes_to_skip", "HPIPE_SIZE_T" );
        if ( beg && next.size() >= 2 )
            es << "if ( ! buf ) goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
        ss << "    HPIPE_DATA.__bytes_to_skip = " << v << " - ( buf->data + buf->used - data );";
        ss << "  t_" << cont_label << ":";
        ss << "    if ( ! buf->next ) {";
        if ( cpp_emitter->interruptible() ) {
            ss << "        if ( last_buf )";
            ss << "            goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
            if ( need_buf_next() ) {
                cpp_emitter->add_variable( "pending_buf", "HPIPE_BUFF_T *" );
                ss << "        HPIPE_DATA.pending_buf = buf;";
                if ( cpp_emitter->buffer_type == CppEmitter::BT_HPIPE_BUFFER ) {
                    if ( need_buf_next() == 1 )
                        ss << "        HPIPE_BUFF_T__INC_REF( buf );";
                    else
                        ss << "        HPIPE_BUFF_T__INC_REF_N( buf, " << need_buf_next() << " );";
                }
                ss << "        HPIPE_DATA.inp_cont = &&e_" << cont_label << ";";
            } else
                ss << "        HPIPE_DATA.inp_cont = &&c_" << cont_label << ";";
            ss << "        return RET_CONT;";
            if ( need_buf_next() ) {
                // e_... (come back code)
                ss << "      e_" << cont_label << ":" << ( cpp_emitter->trace_labels ? " std::cout << \"e_" + to_string( cont_label ) + " \" << __LINE__ << std::endl;" : "" );
                ss << "        HPIPE_DATA.pending_buf->next = buf;";
                ss << "        HPIPE_DATA.pending_buf = buf;";
                ss << "        goto c_" << cont_label << ";";
            }
        } else
            ss << "        goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
        ss << "    }";
        if ( cpp_emitter->buffer_type == CppEmitter::BT_HPIPE_CB_STRING_PTR )
            ss << "    end -= buf->used; buf = buf->next;";
        else if ( need_buf_next() > 1 )
            ss << "    HPIPE_BUFF_T__INC_REF_N( buf, " << need_buf_next() - 1 << " ); buf = buf->next;";
        else if ( need_buf_next() == 1 )
            ss << "    buf = buf->next;";
        else
            ss << "    HPIPE_BUFF_T *old = buf; buf = buf->next; HPIPE_BUFF_T__DEC_REF( old );";
        ss << "  c_" << cont_label << ":";
        ss << "    if ( HPIPE_DATA.__bytes_to_skip >= buf->used ) {";
        ss << "        HPIPE_DATA.__bytes_to_skip -= buf->used;";
        ss << "        goto t_" << cont_label << ";";
        ss << "    }";
        ss << "    data = buf->data + HPIPE_DATA.__bytes_to_skip;";
        if ( cpp_emitter->interruptible() )
            ss << "    end_m1 = buf->data + buf->used - 1;";
        else
            ss << "    end_m1 = buf->data + ( end > buf->used ? buf->used : end ) - 1;";
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

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
    if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER ) {
        unsigned cont_label = ++cpp_emitter->nb_cont_label;

        // cpp_emitter->need_loc_var(  );
        ss << "if ( " << var << " ) {";
        ss << "    sipe_data->__bytes_to_skip = ( " << var << " ) - 1;";
        ss << "    while ( data + sipe_data->__bytes_to_skip > end_m1 ) {";
        ss << "        sipe_data->__bytes_to_skip -= data + sipe_data->__bytes_to_skip - end_m1 - 1;";
        ss << "        if ( last_buf )";
        ss << "            goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
        ss << "        if ( buf->next ) {";
        ss << "            // dec_ref...";
        ss << "            buf    = buf->next;";
        ss << "            continue;";
        ss << "        }";
        ss << "        sipe_data->inp_cont = &&c_" << cont_label << ";";
        ss << "        return RET_CONT;";
        ss << "      c_" << cont_label << ":";
        ss << "        ; // pending ??";
        ss << "    }";
        ss << "    end_m1 = buf->data + buf->used - 1;";
        ss << "    data = buf->data;";
        ss << "    data += sipe_data->__bytes_to_skip;";
        ss << "}";
        ss << "";
    } else if ( cpp_emitter->buffer_type == CppEmitter::C_STR ) {
        if ( beg ) {
            ss << "if ( " << var << " ) {";
            ss.beg += "    ";
        }
        ss << "for( size_t n = ( " << var << ( beg ? " ) - 1" : " )" ) << "; n--; ++data )";
        ss << "    if ( *data == " << cpp_emitter->end_char << " )";
        ss << "        goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
        ss << "if ( *data == " << cpp_emitter->end_char << " )";
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

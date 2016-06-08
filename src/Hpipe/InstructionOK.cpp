#include "InstructionOK.h"
#include "CppEmitter.h"

namespace Hpipe {

void InstructionOK::write_dot( std::ostream &os ) const {
    os << "OK";
}

Instruction *InstructionOK::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    return inst_pool << new InstructionOK( ncx );
}

void InstructionOK::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    if ( cpp_emitter->inst_to_go_if_ok ) {
        // free mark
        if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER and cpp_emitter->rewind_rec_level == 1 )
            ss << "sipe_data->rw_buf->dec_ref_upto( buf );";
        //
        ss << "goto l_" << cpp_emitter->inst_to_go_if_ok->get_id_gen( cpp_emitter ) << ";";
    } else {
        if ( cpp_emitter->interruptible() )
            ss << "sipe_data->inp_cont = &&c_" << ++cpp_emitter->nb_cont_label << ";";
        ss << "return RET_OK;";
        if ( cpp_emitter->interruptible() ) {
            es.rm_beg( 2 ) << "c_" << cpp_emitter->nb_cont_label << ":";
            ss << "return RET_ENDED_OK;";
        }
    }
}


} // namespace Hpipe

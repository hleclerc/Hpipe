#include "InstructionOK.h"
#include "CppEmitter.h"

namespace Hpipe {

void InstructionOK::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "OK";
}

Instruction *InstructionOK::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    return inst_pool << new InstructionOK( ncx );
}

void InstructionOK::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    if ( cpp_emitter->interruptible() )
        ss << "sipe_data->inp_cont = &&c_" << ++cpp_emitter->nb_cont_label << ";";

    ss << "return RET_OK;";

    if ( cpp_emitter->interruptible() ) {
        es.rm_beg( 2 ) << "c_" << cpp_emitter->nb_cont_label << ":";
        es << "return RET_ENDED_OK;";
    }
}


} // namespace Hpipe

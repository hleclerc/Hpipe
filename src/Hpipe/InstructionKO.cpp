#include "InstructionKO.h"
#include "CppEmitter.h"

namespace Hpipe {

void InstructionKO::write_dot( std::ostream &os ) const {
    os << "KO";
}

Instruction *InstructionKO::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    return inst_pool << new InstructionKO( ncx );
}

void InstructionKO::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    if ( cpp_emitter->interruptible() )
        ss << "sipe_data->inp_cont = &&c_" << ++cpp_emitter->nb_cont_label << ";";
    ss << "return RET_KO;";
    if ( cpp_emitter->interruptible() ) {
        es.rm_beg( 2 ) << "c_" << cpp_emitter->nb_cont_label << ":";
        ss << "return RET_ENDED_KO;";
    }
}


} // namespace Hpipe

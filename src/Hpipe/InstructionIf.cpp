#include "InstructionNone.h"
#include "InstructionSave.h"
#include "InstructionIf.h"
#include "FindVarInCode.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionIf::InstructionIf( const Context &cx, const std::string &cond, const CharItem *active_ci ) : InstructionWithCode( cx, active_ci ), cond( cond ) {
}

void InstructionIf::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "IF(" << cond << ")";
}

Instruction *InstructionIf::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    if ( not ncx.pos.contains( active_ci ) )
        return inst_pool << new InstructionNone( ncx );
    return inst_pool << new InstructionIf( ncx, cond, active_ci );
}

bool InstructionIf::same_code( const Instruction *_that ) const {
    const InstructionIf *that = dynamic_cast<const InstructionIf *>( _that );
    return that and cond == that->cond;
}

bool InstructionIf::can_be_deleted() const {
    return false;
}

void InstructionIf::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    ss << "if ( " << cond << " ) goto l_" << next[ 0 ].inst->get_id_gen( cpp_emitter ) << ";";
    write_trans( ss, cpp_emitter, 1 );
}

void InstructionIf::write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    HPIPE_TODO;
    //    if ( not save ) {
    //        PRINTL( "bing" );
    //        return;
    //    }
    //    if ( cpp_emitter->interruptible() )
    //        ss << "sipe_data->" << var << " += sipe_data->__save[ " << save->num_save << " ];";
    //    else
    //        ss << "sipe_data->" << var << " += save[ " << save->num_save << " ];";
}

bool InstructionIf::data_code() const {
    return find_var_in_code( cond, "data", 0 ) != std::string::npos;
}

} // namespace Hpipe

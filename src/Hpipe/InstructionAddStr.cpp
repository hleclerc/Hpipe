#include "InstructionAddStr.h"
#include "InstructionNone.h"
#include "InstructionSave.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionAddStr::InstructionAddStr( const Context &cx, const std::string &var, const CharItem *active_ci ) : InstructionWithCode( cx, active_ci ), var( var ) {
}

void InstructionAddStr::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "AS(" << var << ")";
}

Instruction *InstructionAddStr::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    if ( not ncx.pos.contains( active_ci ) )
        return inst_pool << new InstructionNone( ncx );
    return inst_pool << new InstructionAddStr( ncx, var, active_ci );
}

void InstructionAddStr::get_code_repr( std::ostream &os ) {
    os << "ADD_STR " << var.size() << " " << var;
}

void InstructionAddStr::reg_var( std::function<void (std::string, std::string)> f ) {
    f( "std::string", var );
}

void InstructionAddStr::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    ss << "sipe_data->" << var << " += *data;";
    write_trans( ss, cpp_emitter );
}

void InstructionAddStr::write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    if ( not save ) {
        PRINTLE( "bing" );
        return;
    }
    if ( cpp_emitter->interruptible() && cpp_emitter->rewind_rec_level == 0 )
        ss << "sipe_data->" << var << " += sipe_data->__save[ " << save->num_save << " ];";
    else
        ss << "sipe_data->" << var << " += save[ " << save->num_save << " ];";
}

bool InstructionAddStr::data_code() const {
    return true;
}

} // namespace Hpipe

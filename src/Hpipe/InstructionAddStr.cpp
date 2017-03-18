#include "InstructionAddStr.h"
#include "InstructionNone.h"
#include "InstructionSave.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionAddStr::InstructionAddStr( const Context &cx, const std::string &var, int num_active_item ) : InstructionWithCode( cx, num_active_item ), var( var ) {
}

void InstructionAddStr::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "AS(" << var << ")";
}

Instruction *InstructionAddStr::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    int ind = ncx.pos.index_first( cx.pos[ num_active_item ] );
    if ( ind >= 0 )
        return inst_pool << new InstructionAddStr( ncx, var, ind );
    return inst_pool << new InstructionNone( ncx );
}

void InstructionAddStr::get_code_repr( std::ostream &os ) {
    os << "ADD_STR " << var.size() << " " << var;
}

void InstructionAddStr::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    ss << "HPIPE_DATA." << var << " += *data;";
    write_trans( ss, cpp_emitter );
}

void InstructionAddStr::write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter, std::string repl_data, std::string repl_buf ) {
    cpp_emitter->add_variable( var, "std::string" );

    ss << "HPIPE_DATA." << var << " += *" << repl_data << ";";

    //    if ( not save ) {
    //        PRINTLE( "bing" );
    //        return;
    //    }
    //    if ( cpp_emitter->interruptible() )
    //        ss << "HPIPE_DATA." << var << " += HPIPE_DATA.__save[ " << save->num_save << " ];";
    //    else
    //        ss << "HPIPE_DATA." << var << " += save[ " << save->num_save << " ];";
}

bool InstructionAddStr::data_code() const {
    return true;
}

} // namespace Hpipe

#include "InstructionFreeStr.h"
#include "InstructionNone.h"
#include "InstructionSave.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionFreeStr::InstructionFreeStr( const Context &cx, const Vec<std::string> &strs ) : Instruction( cx ), strs( strs ) {
}

void InstructionFreeStr::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "FREE_STR(" << strs << ")";
}

Instruction *InstructionFreeStr::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    return inst_pool << new InstructionFreeStr( ncx, strs );
}

void InstructionFreeStr::get_code_repr( std::ostream &os ) {
    os << "FREE_STR " << strs.size() << " " << strs;
}

void InstructionFreeStr::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    write_cpp_code_seq( ss, es, cpp_emitter );
    write_trans( ss, cpp_emitter );
}

void InstructionFreeStr::write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter, std::string repl_data, std::string repl_buf ) {
    if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER ) {
        for( const std::string &var : strs )
            ss << "HPIPE_BUFFER::skip( sipe_data->__beg_" << var << "_buf, sipe_data->__beg_" << var << "_data, HPIPE_BUFFER::size_between( sipe_data->__beg_" << var << "_buf, sipe_data->__beg_" << var << "_data, " << repl_buf << ", " << repl_data << " ) );";
    }
}

bool InstructionFreeStr::data_code() const {
    return true;
}

bool InstructionFreeStr::can_be_deleted() const {
    return mark;
}

} // namespace Hpipe

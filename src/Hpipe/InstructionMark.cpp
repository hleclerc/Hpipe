#include "InstructionMark.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionMark::InstructionMark( const Context &cx, unsigned num_active_item ) : Instruction( cx ), num_active_item( num_active_item ) {
    mark                = this;
    has_code            = false;
    has_ambiguous_code  = false;
    has_code_in_a_cycle = false;
}

void InstructionMark::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "MARK" << num_active_item;
}

bool InstructionMark::is_a_mark() const {
    return true;
}

void InstructionMark::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    int rewind_rec_level = cpp_emitter->rewind_rec_level;
    if ( cpp_emitter->interruptible() and not rewind_rec_level ) {
        if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER ) {
            ss << "sipe_data->pending_buf = buf;";
            ss << "sipe_data->rw_buf = buf;";
        }
        ss << "sipe_data->rw_ptr = data;";
    } else {
        rewind_rec_level -= cpp_emitter->interruptible();

        ss << "rw_ptr[ " << rewind_rec_level << " ] = data;";
        if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER )
            ss << "rw_buf[ " << rewind_rec_level << " ] = buf;";
    }
}

Transition *InstructionMark::train( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous ) {
    m = s;
    return &next[ 0 ];
}

bool InstructionMark::with_code() const {
    return true;
}


Instruction *InstructionMark::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    return inst_pool << new InstructionMark( ncx, num_active_item );
}

} // namespace Hpipe

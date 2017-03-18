#include "InstructionMark.h"
#include "InstructionSkip.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionMark::InstructionMark( const Context &cx, unsigned num_active_item ) : Instruction( cx ), num_active_item( num_active_item ) {
    mark = this;
}

void InstructionMark::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "MARK"; // << num_active_item;
}

void InstructionMark::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    if ( cpp_emitter->interruptible() ) {
        if ( cpp_emitter->buffer_type == CppEmitter::BT_HPIPE_BUFFER ) {
            cpp_emitter->add_variable( "pending_buf", "HPIPE_BUFF_T *"  );
            cpp_emitter->add_variable( "rw_buf", "HPIPE_BUFF_T *"  );
            ss << "HPIPE_DATA.pending_buf = buf;";
            ss << "HPIPE_DATA.rw_buf = buf;";
        }
        cpp_emitter->add_variable( "rw_ptr", "const HPIPE_CHAR_T *" );
        ss << "HPIPE_DATA.rw_ptr = data;";
    } else {
        cpp_emitter->loc_vars.push_back_unique( "const HPIPE_CHAR_T *rw_ptr;"  );
        ss << "rw_ptr = data;";
        if ( cpp_emitter->buffer_type == CppEmitter::BT_HPIPE_BUFFER ) {
            cpp_emitter->loc_vars.push_back_unique( "HPIPE_BUFF_T *rw_buf;"  );
            ss << "rw_buf = buf;";
        }
    }
}

Transition *InstructionMark::train( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous ) {
    m = s;
    return &next[ 0 ];
}

bool InstructionMark::with_code() const {
    return true;
}

void InstructionMark::get_code_repr( std::ostream &os ) {
    os << "MARK " << num_active_item;
}

Instruction *InstructionMark::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    // return inst_pool << new InstructionMark( ncx, num_active_item );
    return inst_pool << new InstructionSkip( ncx );
}

} // namespace Hpipe

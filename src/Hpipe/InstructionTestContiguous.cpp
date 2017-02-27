#include "InstructionTestContiguous.h"
#include "InstructionNone.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionTestContiguous::InstructionTestContiguous(const Context &cx, bool beg, unsigned nb_chars ) : Instruction( cx ), beg( beg ), nb_chars( nb_chars ) {
}

void InstructionTestContiguous::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "+" << nb_chars << "?";
    if ( beg )
        os << "(B)";
}

Transition *InstructionTestContiguous::train( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous ) {
    if ( use_contiguous and s + nb_chars - beg < inp.size() ) {
        s += nb_chars - beg;
        return &next[ 0 ];
    }
    return &next[ 1 ];
}

Instruction *InstructionTestContiguous::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    if ( keep_ind.size() == 1 and keep_ind[ 0 ] == 1 )
        return inst_pool << new InstructionNone( ncx );
    return inst_pool << new InstructionTestContiguous( ncx, beg, nb_chars );
}

void InstructionTestContiguous::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    if ( next.size() == 1 ) {
        ss << "data += " << nb_chars - beg << ";";
        return write_trans( ss, cpp_emitter );
    }
    
    if ( cpp_emitter->buffer_type == CppEmitter::C_STR ) {
        if ( next[ 1 ].inst->num_ordering != num_ordering + 1 )
            ss << "goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
    } else {
        ss << "if ( data + " << nb_chars - beg << " > end_m1 ) goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";
        ss << "data += " << nb_chars - beg << ";";
        write_trans( ss, cpp_emitter );
    }
}

void InstructionTestContiguous::get_code_repr( std::ostream &os ) {
    os << "TEST_CONTIGUOUS " << beg << " " << nb_chars;
}


} // namespace Hpipe

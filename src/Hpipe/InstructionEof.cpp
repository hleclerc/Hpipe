#include "InstructionNone.h"
#include "InstructionEof.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionEof::InstructionEof( const Context &cx, bool beg ) : Instruction( cx ), beg( beg ) {
}

void InstructionEof::write_dot( std::ostream &os ) const {
    os << "EOF";
    if ( beg )
        os << "(B)";
}

Instruction *InstructionEof::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    if ( keep_ind.size() == 1 )
        return inst_pool << new InstructionNone( ncx );
    return inst_pool << new InstructionEof( ncx, beg );
}

void InstructionEof::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    // eof test
    ss << "if ( "
       << ( cpp_emitter->buffer_type == CppEmitter::C_STR ? "data[ " + to_string( 1 - beg ) + " ] == " + to_string( cpp_emitter->end_char ) : "data " + std::string( beg ? ">" : ">=" ) + " end_m1" )
       << ( cpp_emitter->interruptible() ? " and last_buf" : "" )
       << " ) goto l_" << next[ 1 ].inst->get_id_gen( cpp_emitter ) << ";";

    // else
    if ( next[ 0 ].inst->num_ordering != num_ordering + 1 )
        ss << "goto l_" << next[ 0 ].inst->get_id_gen( cpp_emitter ) << ";";
}

Transition *InstructionEof::train( std::string::size_type &s, std::string::size_type &m, const std::string &inp, double freq, bool use_contiguous ) {
    return &next[ s == inp.size() ];
}

} // namespace Hpipe

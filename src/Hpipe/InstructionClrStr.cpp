#include "InstructionClrStr.h"
#include "InstructionNone.h"

namespace Hpipe {

InstructionClrStr::InstructionClrStr( const Context &cx, const std::string &var, const CharItem *active_ci ) : InstructionWithCode( cx, active_ci ), var( var ) {
}

void InstructionClrStr::write_dot( std::ostream &os ) const {
    os << "CS(" << var << ")";
}

Instruction *InstructionClrStr::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    if ( not ncx.pos.contains( active_ci ) )
        return inst_pool << new InstructionNone( ncx );
    return inst_pool << new InstructionClrStr( ncx, var, active_ci );
}

bool InstructionClrStr::same_code( const Instruction *_that ) const {
    const InstructionClrStr *that = dynamic_cast<const InstructionClrStr *>( _that );
    return that and var == that->var;
}

void InstructionClrStr::reg_var( std::function<void (std::string, std::string)> f ) {
    f( "std::string", var );
}

void InstructionClrStr::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    ss << "sipe_data->" << var << ".clear();";
    write_trans( ss, cpp_emitter );
}

void InstructionClrStr::write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    ss << "sipe_data->" << var << ".clear();";
}

} // namespace Hpipe

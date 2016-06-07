#include "InstructionCode.h"
#include "InstructionNone.h"
#include "InstructionSave.h"
#include "FindVarInCode.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionCode::InstructionCode( const Context &cx, const std::string &code, const CharItem *active_ci ) : InstructionWithCode( cx, active_ci ), code( code ) {
}

void InstructionCode::write_dot( std::ostream &os ) const {
    os << ( code.size() > 12 ? code.substr( 0, 9 ) + "..." : code );
}

Instruction *InstructionCode::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    if ( not ncx.pos.contains( active_ci ) )
        return inst_pool << new InstructionNone( ncx );
    return inst_pool << new InstructionCode( ncx, code, active_ci );
}

bool InstructionCode::same_code( const Instruction *_that ) const {
    const InstructionCode *that = dynamic_cast<const InstructionCode *>( _that );
    return that and code == that->code;
}

void InstructionCode::write_code( StreamSepMaker &ss, CppEmitter *cpp_emitter, std::string str, const std::string &repl ) {
    for( auto &p : cpp_emitter->variables ) {
        for( std::string::size_type pos = 0; ; ) {
            pos = find_var_in_code( str, p.first, pos );
            if ( pos == std::string::npos )
                break;
            str = str.replace( pos, 0, "sipe_data->" );
            pos += 11 + p.first.size();
        }
    }
    if ( repl.size() )
        str = cpp_emitter->repl_data( str, repl );
    ss << str;
}

void InstructionCode::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    write_code( ss, cpp_emitter, code );
    write_trans_0( ss, cpp_emitter );
}

void InstructionCode::write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    std::string repl_data;
    if ( save )
        repl_data = "( " + std::string( cpp_emitter->rewind_rec_level or not cpp_emitter->interruptible() ? "" : "sipe_data->__" ) + "save + " + to_string( save->num_save ) + " )";

    write_code( ss, cpp_emitter, code, repl_data );
}

bool InstructionCode::data_code() const {
    return find_var_in_code( code, "data", 0 ) != std::string::npos;
}

} // namespace Hpipe

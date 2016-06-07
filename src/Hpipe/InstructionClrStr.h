#pragma once

#include "InstructionWithCode.h"

namespace Hpipe {

/**
*/
class InstructionClrStr : public InstructionWithCode {
public:
    InstructionClrStr( const Context &cx, const std::string &var, const CharItem *active_ci );
    virtual void         write_dot         ( std::ostream &os ) const;
    virtual void         reg_var           ( std::function<void (std::string, std::string)> f );
    virtual void         write_cpp         ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual void         write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual Instruction *clone             ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual bool         same_code         ( const Instruction *that ) const;

    std::string var;
};

} // namespace Hpipe

#pragma once

#include "InstructionWithCode.h"

namespace Hpipe {

/**
*/
class InstructionIf : public InstructionWithCode {
public:
    InstructionIf( const Context &cx, const std::string &cond, const CharItem *active_ci );

    virtual void         write_dot         ( std::ostream &os ) const;
    virtual void         write_cpp         ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual void         write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual bool         data_code         () const;
    virtual Instruction *clone             ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual bool         same_code         ( const Instruction *that ) const;
    virtual bool         can_be_deleted    () const;

    std::string          cond;
};

} // namespace Hpipe
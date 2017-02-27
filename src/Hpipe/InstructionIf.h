#pragma once

#include "InstructionWithCode.h"

namespace Hpipe {

/**
*/
class InstructionIf : public InstructionWithCode {
public:
    InstructionIf( const Context &cx, const std::string &cond, const CharItem *active_ci );

    virtual void         write_dot         ( std::ostream &os, std::vector<std::string> *edge_labels = 0 ) const;
    virtual void         write_cpp         ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual void         write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter );
    virtual bool         data_code         () const;
    virtual Instruction *clone             ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind );
    virtual bool         can_be_deleted    () const;
    virtual void         get_code_repr( std::ostream &os ) override;

    std::string          cond;
};

} // namespace Hpipe

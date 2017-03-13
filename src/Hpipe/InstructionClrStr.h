#pragma once

#include "InstructionWithCode.h"

namespace Hpipe {

/**
*/
class InstructionClrStr : public InstructionWithCode {
public:
    InstructionClrStr( const Context &cx, const std::string &var, int num_active_item );
    virtual void         write_dot         ( std::ostream &os, std::vector<std::string> *edge_labels = 0 ) const override;
    virtual void         reg_var           ( std::function<void (std::string, std::string)> f, CppEmitter *cpp_emitter ) override;
    virtual void         write_cpp         ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) override;
    virtual void         write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter, std::string repl_data = {} ) override;
    virtual Instruction *clone             ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) override;
    virtual void         get_code_repr     ( std::ostream &os ) override;

    std::string var;
};

} // namespace Hpipe

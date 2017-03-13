#pragma once

#include "InstructionWithCode.h"

namespace Hpipe {

/**
*/
class InstructionCode : public InstructionWithCode {
public:
    InstructionCode( const Context &cx, const std::string &code, int num_active_item );
    virtual void         write_dot         ( std::ostream &os, std::vector<std::string> *edge_labels = 0 ) const override;
    virtual void         write_cpp         ( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) override;
    virtual void         write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter, std::string repl_data = "data", std::string repl_buf = "buf" ) override;
    virtual bool         data_code         () const override;
    virtual Instruction *clone             ( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) override;
    virtual bool         has_ret_cont      () const override;
    static void          write_code        ( StreamSepMaker &ss, CppEmitter *cpp_emitter, std::string str, const std::string &repl = {} );
    virtual void         get_code_repr     ( std::ostream &os ) override;

    std::string          code;
};

} // namespace Hpipe

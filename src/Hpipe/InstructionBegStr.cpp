#include "InstructionBegStr.h"
#include "InstructionNone.h"
#include "InstructionSave.h"
#include "CppEmitter.h"

namespace Hpipe {

InstructionBegStr::InstructionBegStr( const Context &cx, const std::string &var, int num_active_item, bool want_next_char ) : InstructionWithCode( cx, num_active_item ), var( var ), want_next_char( want_next_char ) {
}

void InstructionBegStr::write_dot( std::ostream &os, std::vector<std::string> *edge_labels ) const {
    os << "BEG_STR(" << var << ")";
}

Instruction *InstructionBegStr::clone( PtrPool<Instruction> &inst_pool, const Context &ncx, const Vec<unsigned> &keep_ind ) {
    int ind = ncx.pos.index_first( cx.pos[ num_active_item ] );
    if ( ind >= 0 )
        return inst_pool << new InstructionBegStr( ncx, var, ind, want_next_char );
    return inst_pool << new InstructionNone( ncx );
}

void InstructionBegStr::get_code_repr( std::ostream &os ) {
    os << "BEG_STR " << var.size() << " " << var;
}

void InstructionBegStr::update_running_strings( std::set<std::string> &strs ) const {
    strs.insert( var );
}

void InstructionBegStr::reg_var( std::function<void (std::string, std::string)> f, CppEmitter *cpp_emitter ) {
    if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER ) {
        f( "const unsigned char *",  "__beg_" + var + "_data" );
        f( "unsigned"             ,  "__beg_" + var + "_off"  );
        f( "HPIPE_BUFFER *"       ,  "__beg_" + var + "_buf"  );
        f( "Hpipe::CbString"      ,  var                      );
    } else {
        f( "const unsigned char *",  "__beg_" + var + "_data" );
        f( "Hpipe::CmString"      ,  var                      );
    }
}

void InstructionBegStr::write_cpp( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter ) {
    write_cpp_code_seq( ss, es, cpp_emitter );
    write_trans( ss, cpp_emitter );
}

void InstructionBegStr::write_cpp_code_seq( StreamSepMaker &ss, StreamSepMaker &es, CppEmitter *cpp_emitter, std::string repl_data, std::string repl_buf ) {
    if ( cpp_emitter->buffer_type == CppEmitter::HPIPE_BUFFER ) {
        ss << "sipe_data->__beg_" << var << "_off = " << want_next_char - bool( cx.flags & cx.FL_BEG ) << ";";
        ss << "sipe_data->__beg_" << var << "_buf = " << repl_buf << ";";
        ss << "sipe_data->__beg_" << var << "_data = " << repl_data << ";";
    } else {
        ss << "sipe_data->__beg_" << var << "_data = " << repl_data << " + " << want_next_char - bool( cx.flags & cx.FL_BEG ) << ";";
    }
}

bool InstructionBegStr::data_code() const {
    return true;
}

} // namespace Hpipe

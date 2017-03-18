#pragma once

#include "InstructionGraph.h"

namespace Hpipe {

/**
*/
class CppEmitter {
public:
    enum {
        BT_HPIPE_BUFFER,
        BT_BEG_END,
        BT_C_STR,
    };
    struct Variable {
        std::string default_value;
        std::string type;
    };
    using VariableMap = std::unordered_map<std::string,Variable>;

    CppEmitter();

    void                  read                ( InstructionGraph *sg );

    void                  write_preliminaries ( StreamSepMaker &ss );
    void                  write_declarations  ( StreamSepMaker &ss );
    void                  write_definitions   ( StreamSepMaker &ss );

    int                   test                ( const std::vector<Lexer::TestData> &tds );
    bool                  bench               ( const std::vector<Lexer::TrainingData> &tds );

    // attributes to be modified before read()
    int                   stop_char;         ///< used if type == C_STR
    int                   buffer_type;       ///< BT_...
    bool                  trace_labels;      ///<
    std::string           hpipe_struct_name; ///<

    // helpers for Instruction*::...
    bool                  interruptible       () const;
    std::string           repl_data           ( std::string code, const std::string &repl );
    void                  need_loc_var        ( const std::string &str );

    bool                  need_bytes_to_skip;
    bool                  need_pending_buf;
    unsigned              size_save_glo;
    unsigned              size_save_loc;
    std::set<std::string> preliminaries;
    bool                  need_mark;
    VariableMap           variables;
    std::set<std::string> includes;
    std::set<std::string> loc_vars;

protected:
    void                  write_constants     ( StreamSepMaker &ss );
    void                  write_hpipe_data    ( StreamSepMaker &ss, const std::string &name = "HpipeData" );
    void                  write_parse_decl    ( StreamSepMaker &ss, const std::string &hpipe_data_name = "HpipeData", const std::string &func_name = "parse", const char *additional_args = 0 );
    void                  write_parse_def     ( StreamSepMaker &ss, const std::string &hpipe_data_name = "HpipeData", const std::string &func_name = "parse", const char *additional_args = 0 );
    void                  write_parse_body    ( StreamSepMaker &ss, Instruction *root );

    void                  get_ordering        ( Vec<Instruction *> &ordering, Instruction *inst );

    // contextual variables
    unsigned              nb_id_gen;
    unsigned              nb_cont_label;
    std::ostringstream    beg_parse_content;
    std::ostringstream    end_parse_content;
};

} // namespace Hpipe

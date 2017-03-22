#pragma once

#include "InstructionGraph.h"

namespace Hpipe {

/**
*/
class CppEmitter {
public:
    using VariableMap = CharGraph::VariableMap;
    using Variable = CharGraph::Variable;

    enum {
        BT_HPIPE_CB_STRING_PTR,
        BT_HPIPE_BUFFER,
        BT_BEG_END,
        BT_C_STR,
    };

    CppEmitter();

    void                  read               ( InstructionGraph *sg );

    // functions to be called after read
    void                  write_preliminaries( StreamSepMaker &ss );
    void                  write_declarations ( StreamSepMaker &ss );
    void                  write_definitions  ( StreamSepMaker &ss );

    int                   test               ( const std::vector<Lexer::TestData> &tds );
    bool                  bench              ( const std::vector<Lexer::TrainingData> &tds );

    // attributes to be modified before read()
    int                   stop_char;         ///< used if type == C_STR
    int                   buffer_type;       ///< BT_...
    bool                  trace_labels;      ///<

    // helpers for Instruction*::...
    bool                  interruptible      () const;
    bool                  need_buf           () const;
    std::string           repl_data          ( std::string code, const std::string &repl );
    void                  write_label        ( StreamSepMaker &ss, unsigned num, char letter = 'l' );
    void                  add_variable       ( const std::string &name, const std::string &type, const std::string &default_value = {} );

    unsigned              nb_cont_label;
    unsigned              size_save_glo;
    unsigned              size_save_loc;
    unsigned              nb_id_gen;

    Vec<std::string>      preliminaries;
    VariableMap           variables;
    Vec<std::string>      includes;
    Vec<std::string>      loc_vars;
    Vec<std::string>      methods;

protected:
    void                  write_parse_def    ( StreamSepMaker &ss, const std::string &hpipe_data_name = "HpipeData", const std::string &func_name = "parse", const char *additional_args = 0 );
    void                  get_ordering       ( Vec<Instruction *> &ordering, Instruction *inst );

    std::string           parse_content;
    std::string           ctor;
};

} // namespace Hpipe

#pragma once

#include "InstructionGraph.h"

namespace Hpipe {

/**
*/
class CppEmitter {
public:
    enum {
        HPIPE_BUFFER,
        BEGEND,
        C_STR,
    };
    struct Variable {
        std::string type;
    };
    using VariableMap = std::unordered_map<std::string,Variable>;

    CppEmitter( InstructionGraph *sg );

    void              write_preliminaries ( StreamSepMaker &ss );
    void              write_constants     ( StreamSepMaker &ss );
    void              write_hpipe_data    ( StreamSepMaker &ss, const std::string &name = "HpipeData" );
    void              write_parse_decl    ( StreamSepMaker &ss, const std::string &hpipe_data_name = "HpipeData", const std::string &func_name = "parse", const char *additional_args = 0 );
    void              write_parse_def     ( StreamSepMaker &ss, const std::string &hpipe_data_name = "HpipeData", const std::string &func_name = "parse", const char *additional_args = 0 );

    int               test                ( const std::vector<Lexer::TestData> &tds );
    bool              bench               ( const std::vector<Lexer::TrainingData> &tds, int type = BEGEND );

    void              write_parse_body    ( StreamSepMaker &ss, Instruction *root );
    std::string       repl_data           ( std::string code, const std::string &repl );

    bool              interruptible       () const { return buffer_type == HPIPE_BUFFER; }

    // data filled by the constructor, from analysis of the graph
    Instruction      *inst_to_go_if_ok;
    unsigned          max_mark_level;
    unsigned          size_save_glo;
    unsigned          size_save_loc;
    VariableMap       variables;

    // attributes that change the behavior of writer ()
    int               end_char;       ///< used if type == C_STR
    bool              test_mode;      ///<
    int               buffer_type;    ///< BUFFER, ...
    bool              in_class;       ///< 
    bool              trace_labels;   ///< 

    // contextual variables
    unsigned          nb_id_gen;
    unsigned          nb_cont_label;
    unsigned          rewind_rec_level;

protected:
    void              get_ordering        ( Vec<Instruction *> &ordering, Instruction *inst );

    Instruction      *root;
    InstructionGraph *sg;
};

} // namespace Hpipe

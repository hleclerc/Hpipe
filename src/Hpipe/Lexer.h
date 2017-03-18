#pragma once

#include "ErrorList.h"
#include "Lexem.h"
#include "Pool.h"
#include "Vec.h"

namespace Hpipe {

/**
*/
class Lexer {
public:
    struct TestData {
        void write_to_stream( std::ostream &os ) const;
        std::string name;
        std::string inp;
        std::string out;
    };
    struct TrainingData {
        void write_to_stream( std::ostream &os ) const;

        std::string name;
        double      freq;
        std::string inp;
    };

    Lexer( ErrorList &error_list );

    Lexem                *read           ( Source *source ); ///< append the data from source

    const Lexem          *find_machine   ( const std::string &name ) const; ///< find the last machine named $name
    const Lexem          *find_machine   ( const Lexem *&args, const std::string &name ) const; ///< find the last machine named $name
    const Lexem          *root           () const; ///< first lexem in the graph (may be null)

    void                  write_to_stream( std::ostream &os ) const;
    bool                  err            ( const Lexem *l, const char *msg ) const;
    Vec<TestData>         test_data      () const;
    Vec<TrainingData>     training_data  () const;
    std::string           methods        () const;

protected:
    const Lexem          *base           () const; ///< base lexem
    int                   read_tok       ( Source *source, const char *cur ); ///< return increment

    int                   push_cpp       ( Source *source, const char *cur ); ///<
    int                   push_str       ( Source *source, const char *cur, char lim ); ///< for "..." or '...'
    int                   push_tok       ( Source *source, const char *beg, const char *end, Lexem::Type type, int grp = -1, char delim = 0 ); ///< return increment

    bool                  assemble_rarg  ( Lexem *o );
    bool                  assemble_larg  ( Lexem *o );
    bool                  assemble_barg  ( Lexem *o, int need_left = true, int need_right = true );

    enum { max_op_grp = 16 };

    Lexem                *sibling[ max_op_grp ];
    Lexem                 first_item, *last;
    std::vector<Lexem *>  stack_last;
    ErrorList            &error_list;
    Pool<Lexem>           lexem_pool;
    int                   num_grp_bracket;
};

} // namespace Hpipe

#pragma once

#include "Stream.h"
#include "Source.h"
#include "Vec.h"

namespace Hpipe {

/**
*/
class Lexem {
public:
    typedef enum {
        OPERATOR,
        VARIABLE,
        TRAINING,
        METHODS,
        STRING,
        NUMBER,
        FLAGS,
        TEST,
        CODE,
        NONE,
    } Type;

    Lexem( Type type, Source *source, const char *beg, const std::string str );

    void        write_to_stream( std::ostream &os ) const;
    int         display_dot    ( const char *f = ".res.dot", const char *prg = 0 ) const;
    int         write_dot      ( StreamSepMaker &os, int p ) const;
    int         ascii_val      () const;

    bool        eq             ( Type type, const std::string &str ) const;
    bool        eq             ( const std::string &str ) const;
    int         to_int         () const;

    const char *beg;    ///< in source
    std::string str;
    Type        type;
    Source     *source;
    Lexem      *children[ 2 ], *parent, *next, *prev, *sibling;
};

std::ostream &operator<<( std::ostream &os, const Lexem &l );

const Lexem *last( const Lexem *l );

} // namespace Hpipe

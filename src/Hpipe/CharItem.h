#pragma once

#include "CharEdge.h"
#include "AutoPtr.h"
#include "Cond.h"
#include "Vec.h"

#include <functional>
#include <set>

namespace Hpipe {

/**
*/
class CharItem {
public:
    enum {
        NEXT_CHAR, ///< go to next char
        ADD_STR,
        CLR_STR,
        BEG_STR,
        END_STR,
        BEGIN,
        PIVOT,
        LABEL,
        COND,
        CODE,
        _EOF,
        _IF,
        KO,
        OK
    };

    CharItem( int type, std::string str );
    CharItem( int type );
    CharItem( Cond cond );

    bool              apply_rec         ( std::function<bool( CharItem * )> f );
    void              get_possible_paths( Vec<Vec<CharItem *>> &paths, std::function<bool( CharItem *item )> f );
    void              write_to_stream   ( std::ostream &os ) const;
    std::string       compact_repr      () const;
    void              write_dot_rec     ( std::ostream &os ) const;
    bool              code_like         () const;
    bool              advancer          () const;

    int               type;
    Vec<CharEdge>     edges;
    Cond              cond;       ///< used if type==COND
    std::string       str;        ///< if type==CODE, str = the code. if ADD_STR, CLR_STR, ...
    mutable unsigned  op_id;
    mutable int       op_mp;
    static unsigned   cur_op_id;
    unsigned          display_id;
};

} // namespace Hpipe

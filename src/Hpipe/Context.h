#pragma once

#include "CharEdge.h"
#include "CharItem.h"
#include "AutoPtr.h"
#include "Assert.h"
#include "Cond.h"
#include "Vec.h"

namespace Hpipe {
class InstructionMark;

/**
*/
class Context {
public:
    using PC = std::pair<Context,Vec<unsigned>>;
    enum {
        BEG = 1,
        ON_EOF = 2,
        NOT_EOF = 4,
    };

    Context( InstructionMark *mark, int flags );
    Context( const CharItem *item, int flags );
    Context();

    bool                       operator<       ( const Context &that ) const;
    void                       write_to_stream ( std::ostream &os ) const;

    PC                         forward         ( const CharItem *fip ) const; ///< replace item by item.edges[*].item
    PC                         forward         ( const Cond &c ) const; ///< replace item by item.edges[*].item
    PC                         forward         ( int type ) const; ///< replace item by item.edges[*].item
    PC                         only_with       ( int type ) const; ///< replace item by item.edges[*].item
    PC                         without          ( const CharItem *fip ) const; ///<
    PC                         keep_up_to      ( unsigned n ) const; ///< (assuming that all the items are conds)
    PC                         keep_only       ( const Vec<unsigned> &keep ) const;
    PC                         with_mark       ( InstructionMark *inst ) const;

    Context                    without_mark    () const;
    Context                    without_beg     () const;
    Context                    with_eof        () const;
    Context                    with_not_eof    () const;
    Context                    without_eof     () const;
    Context                    without_not_eof () const;

    bool                       only_has        ( int char_item_type ) const;
    bool                       has             ( int char_item_type ) const;
    int                        index_of_first  ( int char_item_type ) const;

    bool                       beg             () const { return flags & BEG; }
    bool                       eof             () const { return flags & ON_EOF; }
    bool                       not_eof         () const { return flags & NOT_EOF; }

    Vec<const CharItem *>      pos;  ///< items, sorted by priority
    InstructionMark           *mark; ///<
    int                        flags;  ///< true if no +1 since the beginning
};

} // namespace Hpipe

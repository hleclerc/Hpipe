#pragma once

#include "CharEdge.h"
#include "CharItem.h"
#include "AutoPtr.h"
#include "Assert.h"
#include "Cond.h"
#include "Vec.h"
#include <map>

namespace Hpipe {
class InstructionMark;

/**
*/
class Context {
public:
    enum { FL_BEG = 1, FL_EOF = 2, FL_NOT_EOF = 4, FL_OK = 8, };
    using PathsToStrings = std::map<std::string,Vec<unsigned>>;
    using PC = std::pair<Context,Vec<unsigned>>;

    Context( const CharItem *item, int flags = 0 );
    Context( int flags = 0 );

    bool                       operator<      ( const Context &that ) const;
    void                       write_to_stream( std::ostream &os ) const;

    PC                         forward        ( const CharItem *fip ) const;          ///< replace item by item.edges[*].item
    PC                         forward        ( const Cond &c ) const;                ///< replace item by item.edges[*].item
    PC                         forward_code   ( unsigned ind ) const;                 ///< replace item by item.edges[*].item
    PC                         forward        ( int type ) const;                     ///< replace item by item.edges[*].item
    PC                         only_with      ( int type ) const;                     ///< replace item by item.edges[*].item
    PC                         without        ( const CharItem *fip ) const;          ///<
    PC                         keep_up_to     ( unsigned n ) const;                   ///< (assuming that all the items are conds)
    PC                         keep_only      ( const Vec<unsigned> &keep ) const;
    PC                         with_mark      ( InstructionMark *mark ) const;        ///<
    PC                         without_mark   () const;                               ///<  const Vec<unsigned> &keep_ind


    Context                    without_flag   ( int val ) const;
    Context                    with_flag      ( int val ) const;
    Context                    without_string ( const std::string &str ) const;
    Context                    with_string    ( const std::string &str ) const;

    void                       add_string     ( const std::string &str, unsigned ind );
    void                       rem_string     ( const std::string &str );

    bool                       only_has       ( int char_item_type ) const;
    bool                       has            ( int char_item_type ) const;
    int                        index_of_first ( int char_item_type ) const;
    bool                       leads_to_ok    ( bool never_ending ) const;
    bool                       beg            () const { return flags & FL_BEG; }
    bool                       eof            () const { return flags & FL_EOF; }
    bool                       not_eof        () const { return flags & FL_NOT_EOF; }

    Vec<const CharItem *>      pos;                                                   ///< items, sorted by priority
    InstructionMark           *mark;                                                  ///<
    Vec<unsigned>              paths_to_mark;                                         ///< for each item in pos, paths_to_mark gives index in mark->cx.pos
    PathsToStrings             paths_to_strings;                                      ///<
    int                        flags;                                                 ///< true if no +1 since the beginning
};

} // namespace Hpipe

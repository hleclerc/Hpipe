#include "CharGraph.h"
#include "CharItem.h"
#include "Context.h"
#include "Assert.h"
#include "DotOut.h"
#include <algorithm>
#include <string.h>
#include <sstream>
#include <limits>

namespace Hpipe {

Context::Context( InstructionMark *mark, int flags ) : mark( mark ), flags( flags ) {
}

Context::Context( const CharItem *item, int flags ) : mark( 0 ), flags( flags ) {
    pos << item;
}

Context::Context() : mark( 0 ), flags( 0 ) {
}

bool Context::operator<( const Context &that ) const {
    return std::tie( pos, mark, flags ) < std::tie( that.pos, that.mark, that.flags );
}

void Context::write_to_stream( std::ostream &os ) const {
    os << pointed_values( pos );
    if ( mark )
        os << " M(" << mark << ")";
    if ( beg() )
        os << " BEG";
    if ( eof() )
        os << " EOF";
    if ( not_eof() )
        os << " NOT_EOF";
}

Context::PC Context::forward( const CharItem *fip ) const {
    Context res( mark, flags );

    Vec<unsigned> rcitem;
    for( unsigned i = 0; i < pos.size(); ++i ) {
        const CharItem *item = pos[ i ];
        if ( item == fip ) {
            for( const CharEdge &e : item->edges ) {
                if ( not res.pos.contains( e.item ) ) {
                    res.pos << e.item;
                    rcitem << i;
                    if ( CharGraph::leads_to_ok( res.pos ) )
                        return { res, rcitem };
                }
            }
        } else if ( not res.pos.contains( item ) ) {
            res.pos << item;
            rcitem << i;
            if ( CharGraph::leads_to_ok( res.pos ) )
                return { res, rcitem };
        }
    }

    return { res, rcitem };
}

Context::PC Context::forward( const Cond &c ) const {
    Context res( mark, flags );

    Vec<unsigned> rcitem;
    for( unsigned i = 0; i < pos.size(); ++i ) {
        const CharItem *item = pos[ i ];
        if ( item->type != CharItem::COND ){
            if ( not res.pos.contains( item ) ) {
                res.pos << item;
                rcitem << i;
                if ( CharGraph::leads_to_ok( res.pos ) )
                    return { res, rcitem };
            }
        } else if ( c & item->cond ) {
            for( const CharEdge &e : item->edges ) {
                if ( not res.pos.contains( e.item ) ) {
                    res.pos << e.item;
                    rcitem << i;
                    if ( CharGraph::leads_to_ok( res.pos ) )
                        return { res, rcitem };
                }
            }
        }
    }

    return { res, rcitem };
}

Context::PC Context::forward( int type ) const {
    Context res( mark, flags );

    Vec<unsigned> rcitem;
    for( unsigned i = 0; i < pos.size(); ++i ) {
        const CharItem *item = pos[ i ];
        if ( item->type == type ) {
            for( const CharEdge &e : item->edges ) {
                if ( not res.pos.contains( e.item ) ) {
                    res.pos << e.item;
                    rcitem << i;
                    if ( CharGraph::leads_to_ok( res.pos ) )
                        return { res, rcitem };
                }
            }
        } else if ( not res.pos.contains( item ) ) {
            res.pos << item;
            rcitem << i;
            if ( CharGraph::leads_to_ok( res.pos ) )
                return { res, rcitem };
        }
    }

    return { res, rcitem };
}

Context::PC Context::only_with( int type ) const {
    Context res( mark, flags );

    Vec<unsigned> rcitem;
    for( unsigned i = 0; i < pos.size(); ++i ) {
        const CharItem *item = pos[ i ];
        if ( item->type == type ) {
            res.pos << item;
            rcitem << i;
        }
    }

    return { res, rcitem };
}

Context::PC Context::without( const CharItem *fip ) const {
    Context res( mark, flags );

    Vec<unsigned> rcitem;
    for( unsigned i = 0; i < pos.size(); ++i ) {
        const CharItem *item = pos[ i ];
        if ( item != fip ) {
            res.pos << item;
            rcitem << i;
        }
    }

    return { res, rcitem };
}

Context::PC Context::keep_up_to( unsigned n ) const {
    Context res( mark, flags );

    for( unsigned i = 0; i <= n; ++i )
        res.pos << pos[ i ];

    return { res, range_vec( n + 1 ) };
}

Context::PC Context::keep_only( const Vec<unsigned> &keep ) const {
    Context res( mark, flags );
    Vec<unsigned> rcitem;

    for( unsigned ind : keep ) {
        res.pos << pos[ ind ];
        rcitem << ind;
    }

    return { res, rcitem };
}

Context::PC Context::with_mark( InstructionMark *inst ) const {
    Context res( inst, flags );
    res.pos = pos;
    return { res, range_vec( unsigned( pos.size() ) ) };
}

void Context::rm_mark() {
    mark = 0;
}

Context Context::without_mark() const {
    Context res( (InstructionMark *)nullptr, flags );
    res.pos = pos;
    return res;
}

Context Context::without_beg() const {
    Context res( mark, flags & ~BEG );
    res.pos = pos;
    return res;
}

Context Context::with_eof() const {
    Context res( mark, flags | ON_EOF );
    res.pos = pos;
    return res;
}

Context Context::with_not_eof() const {
    Context res( mark, flags | NOT_EOF );
    res.pos = pos;
    return res;
}

Context Context::without_eof() const {
    Context res( mark, flags & ~ ON_EOF );
    res.pos = pos;
    return res;
}

Context Context::without_not_eof() const {
    Context res( mark, flags & ~ NOT_EOF );
    res.pos = pos;
    return res;
}

bool Context::only_has( int char_item_type ) const {
    for( const CharItem *item : pos )
        if ( item->type != char_item_type )
            return false;
    return true;
}

bool Context::has( int char_item_type ) const {
    return index_of_first( char_item_type ) >= 0;
}

int Context::index_of_first( int char_item_type ) const {
    for( unsigned i = 0; i < pos.size(); ++i )
        if ( pos[ i ]->type == char_item_type )
            return i;
    return -1;
}

} // namespace Hpipe

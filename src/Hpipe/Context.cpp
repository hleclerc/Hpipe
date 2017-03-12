#include "InstructionMark.h"
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

namespace {

/// helper to construct a new Context::PC
struct PcMaker {
    PcMaker( const Context *orig ) : res{ orig->flags, {} }, orig{ orig } {
        res.first.mark = orig->mark;
    }

    void add( const CharItem *item, unsigned ind, bool with_mark = true ) {
        if ( with_mark && orig->mark )
            res.first.code_path << orig->code_path[ ind ];
        res.first.pos << item;
        res.second << ind;
    }

    Context::PC out() {
        return res;
    }

    bool leads_to_ok() const {
        return CharGraph::leads_to_ok( res.first.pos );
    }

    bool has( const CharItem *item ) const {
        return res.first.pos.contains( item );
    }

    Context::PC    res;
    const Context *orig;
};

}

Context::Context( const CharItem *item, int flags ) : flags( flags ), mark( 0 ) {
    code_path << false;
    pos << item;
}

Context::Context( int flags ) : flags( flags ), mark( 0 ) {
}

bool Context::operator<( const Context &that ) const {
    return std::tie( pos, mark, code_path, flags ) < std::tie( that.pos, that.mark, that.code_path, that.flags );
}

void Context::write_to_stream( std::ostream &os ) const {
    os << pointed_values( pos );
    if ( beg() )
        os << " BEG";
    if ( eof() )
        os << " EOF";
    if ( not_eof() )
        os << " NOT_EOF";
}

Context::PC Context::forward( const CharItem *fip ) const {
    PcMaker pm( this );

    for( unsigned i = 0; i < pos.size(); ++i ) {
        const CharItem *item = pos[ i ];
        if ( item == fip ) {
            for( const CharEdge &e : item->edges ) {
                if ( ! pm.has( e.item ) ) {
                    pm.add( e.item, i );
                    if ( pm.leads_to_ok() )
                        return pm.out();
                }
            }
        } else if ( ! pm.has( item ) ) {
            pm.add( item, i );
            if ( pm.leads_to_ok() )
                return pm.out();
        }
    }

    return pm.out();
}

Context::PC Context::forward( const Cond &c ) const {
    PcMaker pm( this );

    for( unsigned i = 0; i < pos.size(); ++i ) {
        const CharItem *item = pos[ i ];
        if ( item->type != CharItem::COND ){
            if ( not pm.has( item ) ) {
                pm.add( item, i );
                if ( pm.leads_to_ok() )
                    return pm.out();
            }
        } else if ( c & item->cond ) {
            for( const CharEdge &e : item->edges ) {
                if ( not pm.has( e.item ) ) {
                    pm.add( e.item, i );
                    if ( pm.leads_to_ok() )
                        return pm.out();
                }
            }
        }
    }

    return pm.out();
}

Context::PC Context::forward( int type ) const {
    PcMaker pm( this );

    for( unsigned i = 0; i < pos.size(); ++i ) {
        const CharItem *item = pos[ i ];
        if ( item->type == type ) {
            for( const CharEdge &e : item->edges ) {
                if ( not pm.has( e.item ) ) {
                    pm.add( e.item, i );
                    if ( pm.leads_to_ok() )
                        return pm.out();
                }
            }
        } else if ( not pm.has( item ) ) {
            pm.add( item, i );
            if ( pm.leads_to_ok() )
                return pm.out();
        }
    }

    return pm.out();
}

Context::PC Context::only_with( int type ) const {
    PcMaker pm( this );

    for( unsigned i = 0; i < pos.size(); ++i )
        if ( pos[ i ]->type == type )
            pm.add( pos[ i ], i );

    return pm.out();
}

Context::PC Context::without( const CharItem *fip ) const {
    PcMaker pm( this );

    for( unsigned i = 0; i < pos.size(); ++i )
        if ( pos[ i ] != fip )
            pm.add( pos[ i ], i );

    return pm.out();
}

Context::PC Context::keep_up_to( unsigned n ) const {
    PcMaker pm( this );

    for( unsigned i = 0; i <= n; ++i )
        pm.add( pos[ i ], i );

    return pm.out();
}

Context::PC Context::keep_only( const Vec<unsigned> &keep ) const {
    PcMaker pm( this );

    for( unsigned ind : keep )
        pm.add( pos[ ind ], ind );

    return pm.out();
}

Context::PC Context::with_mark( InstructionMark *mark ) const {
    PcMaker pm( this );

    for( unsigned i = 0; i < pos.size(); ++i ) {
        pm.res.first.code_path << i;
        pm.add( pos[ i ], i );
    }

    pm.res.first.mark = mark;
    return pm.out();
}

Context::PC Context::without_mark( const Vec<unsigned> &keep_ind ) const {
    PcMaker pm( this );

    for( unsigned i = 0; i < pos.size(); ++i )
        if ( keep_ind.contains( i ) )
            pm.add( pos[ i ], i, false );

    pm.res.first.flags |= FL_OK;
    pm.res.first.mark = nullptr;
    return pm.out();
}

Context Context::without_flag( int val ) const {
    Context res( *this );
    res.flags &= ~ val;
    return res;
}

Context Context::with_flag(int val) const {
    Context res( *this );
    res.flags |= val;
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

bool Context::leads_to_ok() const {
    return ( flags & FL_OK ) || CharGraph::leads_to_ok( pos );
}

} // namespace Hpipe

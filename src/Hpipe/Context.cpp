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
    PcMaker( const Context *orig, int flags ) : res{ flags, {} }, orig{ orig } {
    }

    PcMaker( const Context *orig ) : PcMaker{ orig, orig->flags } {
    }

    void add( const CharItem *item, unsigned ind ) {
        res.first.pos << item;
        res.second << ind;
    }

    Context::PC out( const CharItem *item = 0, unsigned ind = 0 ) {
        for( const Context::Code &code : orig->codes ) {
            Vec<unsigned> new_ok_paths;
            for( unsigned num_path = 0; num_path < res.second.size(); ++num_path )
                if ( code.ok_paths.contains( res.second[ num_path ] ) )
                    new_ok_paths << num_path;
            if ( new_ok_paths.size() ) {
                Context::Code new_code = code;
                new_code.ok_paths = new_ok_paths;
                res.first.codes << new_code;
            }
        }

        if ( item ) {
            Context::Code *code = res.first.codes.new_elem();
            code->ok_paths << ind;
            code->off_prec = 0;
            code->off_loop = 0;
            code->item = item;
        }

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

Context::Context( const CharItem *item, int flags ) : flags( flags ) {
    pos << item;
}

Context::Context( int flags ) : flags( flags ) {
}

bool Context::operator<( const Context &that ) const {
    return std::tie( pos, codes, flags ) < std::tie( that.pos, that.codes, that.flags );
}

bool Context::Code::operator<( const Code &that ) const {
    return std::tie( item, ok_paths ) < std::tie( that.item, that.ok_paths );
}

void Context::Code::write_to_stream( std::ostream &os ) const {
    os << "(" << off_prec;
    if ( off_loop )
        os << "+" << off_loop << "n";
    os << ")";
    os << *item;
}

void Context::write_to_stream( std::ostream &os ) const {
    os << pointed_values( pos );
    if ( beg() )
        os << " BEG";
    if ( eof() )
        os << " EOF";
    if ( not_eof() )
        os << " NOT_EOF";
    for( const Code &code : codes )
        os << " " << code;
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

Context::PC Context::forward_code( unsigned ind ) const {
    PcMaker pm( this );

    for( unsigned i = 0; i < pos.size(); ++i ) {
        const CharItem *item = pos[ i ];
        if ( i == ind ) {
            for( const CharEdge &e : item->edges ) {
                if ( ! pm.has( e.item ) ) {
                    pm.add( e.item, i );
                    if ( pm.leads_to_ok() )
                        return pm.out( pos[ ind ], ind );
                }
            }
        } else if ( ! pm.has( item ) ) {
            pm.add( item, i );
            if ( pm.leads_to_ok() )
                return pm.out( pos[ ind ], ind );
        }
    }

    return pm.out( pos[ ind ], ind );
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

Context Context::without_beg() const {
    Context res( *this );
    res.flags &= ~BEG;
    return res;
}

Context Context::with_eof() const {
    Context res( *this );
    res.flags |= ON_EOF;
    return res;
}

Context Context::with_not_eof() const {
    Context res( *this );
    res.flags |= NOT_EOF;
    return res;
}

Context Context::without_eof() const {
    Context res( *this );
    res.flags &= ~ON_EOF;
    return res;
}

Context Context::without_not_eof() const {
    Context res( *this );
    res.flags &= ~NOT_EOF;
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

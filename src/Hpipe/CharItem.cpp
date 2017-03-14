#include "CharItem.h"
#include "DotOut.h"
#include "Assert.h"
#include <algorithm>
#include <unordered_map>

namespace Hpipe {

unsigned CharItem::cur_op_id = 0;

CharItem::CharItem( Cond cond ) : CharItem( COND ) {
    this->cond = cond;
}

CharItem::CharItem( int type, std::string str ) : CharItem( type ) {
    this->str = str;
}

CharItem::CharItem( int type ) : type( type ), op_id( 0 ) {
    static unsigned sid = 0;
    display_id = ++sid;
}

bool CharItem::apply_rec( std::function<bool( CharItem * )> f ) {
    if ( op_id == CharItem::cur_op_id )
        return true;
    op_id = CharItem::cur_op_id;

    if ( not f( this ) )
        return false;

    bool res = true;
    for( CharEdge &t : edges )
        if ( not t.item->apply_rec( f ) )
            res = false;
    return res;
}

void CharItem::get_possible_paths( Vec<Vec<CharItem *>> &paths, std::function<bool (CharItem *)> f ) {
    Vec<Vec<CharItem *>> front;
    front << this;
    while ( front.size() ) {
        Vec<CharItem *> path = front.back();
        front.pop_back();

        CharItem *last = path.back();
        if ( last->edges.size() ){
            if ( f( last ) ){
                for( CharEdge &t : last->edges ) {
                    Vec<CharItem *> new_path = path;
                    new_path << t.item;
                    if ( path.contains( t.item ) )
                        paths << new_path;
                    else
                        front << new_path;
                }
            } else
                paths << path;
        } else
            paths << path;
    }
}

void CharItem::write_to_stream( std::ostream &os ) const {
    os << "(" << compact_repr() << ")";
    switch ( type ) {
    case CharItem::END_STR_INCL: os << "EI(" << str << ")"; break;
    case CharItem::NEXT_CHAR:    os << "+1"; break;
    case CharItem::ADD_STR:      os << "AS(" << str << ")"; break;
    case CharItem::CLR_STR:      os << "CL(" << str << ")"; break;
    case CharItem::BEG_STR:      os << "BS(" << str << ")"; break;
    case CharItem::END_STR:      os << "ES(" << str << ")"; break;
    case CharItem::BEGIN:        os << "B"; break;
    case CharItem::PIVOT:        os << "P"; break;
    case CharItem::LABEL:        os << "LABEL"; break;
    case CharItem::COND:         os << cond; break;
    case CharItem::CODE:         os << ( str.size() > 9 ? str.substr( 0, 6 ) + "..." : str ); break;
    case CharItem::_EOF:         os << "EOF"; break;
    case CharItem::_IF:          os << "IF"; break;
    case CharItem::KO:           os << "KO"; break;
    case CharItem::OK:           os << "OK"; break;
    default:                     os << "?";
    }
}

std::string CharItem::compact_repr() const {
    static std::unordered_map<const CharItem *,unsigned> d;
    if ( not d.count( this ) )
        d[ this ] = d.size();
    std::string res;
    unsigned c = d[ this ];
    if ( not c )
        res = '0';
    else {
        while ( c ) {
            res += 'a' + c % 26;
            c /= 26;
        }
        std::reverse( res.begin(), res.end() );
    }
    return res;
}

void CharItem::write_dot_rec( std::ostream &os ) const {
    if ( op_id == CharItem::cur_op_id )
        return;
    op_id = CharItem::cur_op_id;

    os << "  node_" << this << " [label=\"";
    dot_out( os, *this );
    os << "\"";
    //if ( leads_to_ok )
    //    os << ",style=dotted";
    os << "];\n";

    int cpt = 0;
    for( const CharEdge &t : edges ) {
        if ( t.item )
            t.item->write_dot_rec( os );
        os << "  node_" << this << " -> node_" << t.item;
        if ( this->edges.size() >= 2 )
            os << " [label=\"" << cpt++ << "\"]";
        os << ";\n";
    }
}

bool CharItem::code_like() const {
    return type == CODE or type == ADD_STR or type == CLR_STR or type == BEG_STR or type == END_STR or type == END_STR_INCL;
}

bool CharItem::advancer() const {
    return type == NEXT_CHAR;
}

} // namespace Hpipe

#pragma once

#include "Assert.h"
#include <iostream>
#include <vector>

namespace Hpipe {

template<class T>
struct Vec : std::vector<T> {
    using const_iterator = typename std::vector<T>::const_iterator;
    struct Size {};

    Vec() {}
    Vec( T a ) { push_back( a ); }
    Vec( T a, T b ) { push_back( a ); push_back( b ); }
    Vec( Size, unsigned size ) : std::vector<T>( size ) {}
    Vec( const_iterator a, const_iterator b ) : std::vector<T>( a, b ) {}

    void     write_to_stream      ( std::ostream &os ) const;

    Vec     &operator<<           ( const T &val ) { push_back( val ); return *this; }

    Vec     &append               ( const Vec<T> &vec ) { for( const T &val : vec ) push_back( val ); return *this; }

    template<class... Args>
    T       *push_back            ( Args &&...args ) { this->emplace_back( std::forward<Args>( args )... ); return &this->back(); }

    T       *new_item             () { this->emplace_back(); return &this->back(); }

    template<class Arg>
    T       *push_back_unique     ( Arg &&val ) { for( unsigned i = 0; i < this->size(); ++i ) if ( this->operator[]( i ) == val ) return &this->operator[]( i ); return push_back( std::forward<Arg>( val ) ); }

    T       *push_item            ( const T &val ) { this->emplace_back( val ); return &this->back(); }

    T       *new_elem             () { this->emplace_back(); return &this->back(); }

    void     remove_unordered     ( unsigned pos ) { if ( pos + 1 != this->size() ) this->operator[]( pos ) = std::move( this->back() ); this->pop_back(); }
    void     remove               ( unsigned pos ) { this->erase( this->begin() + pos, this->begin() + pos + 1 ); }
    void     remove_duplicates    () { for( unsigned i = 0; i < this->size(); ++i ) for( unsigned j = i + 1; j < this->size(); ++j ) if ( this->operator[]( i ) == this->operator[]( j ) ) remove( j-- ); }

    void     remove_first         ( const T &val ) { for( unsigned i = 0; i < this->size(); ++i ) if ( this->operator[]( i ) == val ) return remove( i ); }
    int      index_first          ( const T &val ) const { for( unsigned i = 0; i < this->size(); ++i ) if ( this->operator[]( i ) == val ) return i; return -1; }

    template<class Op>
    void     remove_first_checking( Op &&op ) { for( unsigned i = 0; i < this->size(); ++i ) if ( op( this->operator[]( i ) ) ) return remove( i ); }

    template<class Op>
    int      index_first_checking ( Op &&op ) { for( unsigned i = 0; i < this->size(); ++i ) if ( op( this->operator[]( i ) ) ) return i; return -1; }

    Vec      subvec               ( unsigned beg, unsigned end ) const { Vec res; res.reserve( end - beg          ); for( unsigned i = beg; i < end         ; ++i ) res << this->operator[]( i ); return res; }
    Vec      without              ( unsigned index             ) const { Vec res; res.reserve( this->size() - 1   ); for( unsigned i = 0  ; i < this->size(); ++i ) if ( i != index ) res << this->operator[]( i ); return res; }
    Vec      up_to                ( unsigned end               ) const { Vec res; res.reserve( end                ); for( unsigned i = 0  ; i < end         ; ++i ) res << this->operator[]( i ); return res; }
    Vec      from                 ( unsigned beg               ) const { Vec res; res.reserve( this->size() - beg ); for( unsigned i = beg; i < this->size(); ++i ) res << this->operator[]( i ); return res; }

    bool     all_eq               () const { for( unsigned i = 1; i < this->size(); ++i ) if ( this->operator[]( i ) != this->operator[]( 0 ) ) return false; return true; }

    const T &operator[]           ( unsigned index ) const { HPIPE_ASSERT_IF_DEBUG( index < this->size() ); return std::vector<T>::operator[]( index ); }
    T       &operator[]           ( unsigned index ) { HPIPE_ASSERT_IF_DEBUG( index < this->size() ); return std::vector<T>::operator[]( index ); }

    void     secure_set                  ( unsigned index, const T &val ) { if ( index >= this->size() ) this->resize( index + 1 ); this->operator[]( index ) = val; }

    bool     contains             ( const T &val ) const { for( unsigned i = 0; i < this->size(); ++i ) if ( this->operator[]( i ) == val ) return true; return false; }

    Vec      concat               ( const Vec &v ) const { Vec res = *this; return res.append( v ); }
};

template<class T>
Vec<T> range_vec( T s ) {
    Vec<T> res;
    res.reserve( s );
    for( T i = 0; i < s; ++i )
        res << i;
    return res;
}

template<class T>
void Vec<T>::write_to_stream( std::ostream &os ) const {
    for( unsigned i = 0; i < this->size(); ++i )
        os << ( i ? " " : "" ) << this->operator[]( i );
}

}

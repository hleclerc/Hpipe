/*
 Copyright 2012 Hugo Leclerc

 This file is part of Hpipe.

 Hpipe is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hpipe is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with Hpipe. If not, see <http://www.gnu.org/licenses/>.
*/


#pragma once

#include "Stream.h"
#include "Vec.h"

#include <bitset>

namespace Hpipe {

/**
*/
class Cond {
public:
    enum { p_size = 256 };

    struct Range {
        Range( int beg, int end ) : beg( beg ), end( end ) {}

        void   write_to_stream( std::ostream &os ) const;
        void   write_cpp      ( std::ostream &os, const std::string &var, bool in_or ) const;
        double cost_cpp       () const;
        int    size           () const { return end + 1 - beg; }

        int      beg;
        int      end; ///< included
        Vec<int> exceptions;
    };
    struct Neg {};

    Cond();
    Cond( const Cond &c );
    Cond( unsigned char c );
    Cond( unsigned char b, unsigned char e ); ///< range [ b, e ]. e is included

    Cond( Neg, const Cond &c );

    operator            bool          () const;

    bool                operator==    ( const Cond &c ) const;
    bool                operator!=    ( const Cond &c ) const;
    bool                operator<     ( const Cond &c ) const;

    Cond               &operator|=    ( const Cond &c );
    Cond               &operator&=    ( const Cond &c );

    Cond                operator|     ( const Cond &c ) const;
    Cond                operator&     ( const Cond &c ) const;

    Cond                operator~     () const; ///< negation


    Cond               &operator<<    ( int v ) { p[ v ] = 1; return *this; }
    bool                operator[]    ( int i ) const { return p[ i ]; }

    Vec<Range>          ok_ranges     () const; ///< as ranges like [ b, e ]
    Vec<Range>          ok_ranges_opt ( const Cond *not_in = 0 ) const; ///< same thing, with stuff like Range::exception
    Vec<Range>          ko_ranges     () const; ///< as ranges like [ b, e ]

    String              ok_cpp        ( String var, const Cond *not_in = 0 ) const; ///< ok condition for cpp
    String              ko_cpp        ( String var, const Cond *not_in = 0 ) const; ///< ok condition for cpp

    double              cost_cpp      ( const Cond *not_in = 0 ) const; ///< no operations to check *this

    int                 nz            () const;
    int                 first_nz      () const;
    bool                included_in   ( const Cond &cond ) const;
    bool                never_checked ( const Cond *not_in = 0 ) const; ///< true
    bool                always_checked( const Cond *not_in = 0 ) const; ///< true if n_ch == 0 or any works
protected:
    String              _cpp( String var, const Cond *not_in = 0 ) const; ///<

    std::bitset<p_size> p;
    // bool             p[ p_size ];

    friend std::ostream &operator<<( std::ostream &os, const Cond &cond );
};

} // namespace Hpipe

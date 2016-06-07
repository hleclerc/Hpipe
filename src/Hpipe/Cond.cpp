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

#include "Assert.h"
#include "Cond.h"

#include <sstream>

namespace Hpipe {

// display of a char
static std::ostream &cc( std::ostream &os, int r ) {
    if ( r == '\\' )
        return os << "'\\\\'";
    if ( r >= 0x20 and r <= 0x7e and r != 39 )
        return os << "'" << char( r ) << "'";
    return os << r;
}

Cond::Cond() {
}

Cond::Cond( const Cond &c ) : p( c.p ) {
}

Cond::Cond( unsigned char c ) {
    p.set( c );
}

Cond::Cond( unsigned char b, unsigned char e ) {
    for( int i = b; i <= e; ++i )
        p.set( i );
}

Cond::Cond( Cond::Neg, const Cond &c ) : p( ~ c.p ) {
}

Cond::operator bool() const {
    return p.any();
}

bool Cond::operator==( const Cond &c ) const {
    return p == c.p;
}

bool Cond::operator!=( const Cond &c ) const {
    return p != c.p;
}

Cond &Cond::operator|=( const Cond &c ) {
    p |= c.p;
    //    for( int i = 0; i < p_size; ++i )
    //        p[ i ] = p[ i ] or c.p[ i ];
    return *this;
}

Cond &Cond::operator&=( const Cond &c ) {
    p &= c.p;
    //    for( int i = 0; i < p_size; ++i )
    //        p[ i ] = p[ i ] and c.p[ i ];
    return *this;
}

Cond Cond::operator|( const Cond &c ) const {
    Cond res = *this;
    res |= c;
    return res;
}

Cond Cond::operator&( const Cond &c ) const {
    Cond res = *this;
    res &= c;
    return res;
}

Cond Cond::operator~() const {
    return { Neg(), *this };
}

Vec<Cond::Range> Cond::ok_ranges() const {
    Vec<Range> res;
    for( int i = 0; i < p_size; ++i ) {
        // beginning if a Range
        if ( p[ i ] ) {
            for( int beg = i; ; ) {
                if ( ++i == p_size ) {
                    res << Range( beg, i - 1 );
                    return res;
                }
                if ( not p[ i ] ) {
                    res << Range( beg, i - 1 );
                    break;
                }
            }
        }
    }

    return res;
}

Vec<Cond::Range> Cond::ok_ranges_opt( const Cond *not_in ) const {
    // extensions with not_in
    Cond tmp = *this;
    if ( not_in )
        tmp &= ~ *not_in;
    Vec<Range> ra = tmp.ok_ranges();

    if ( not_in ) {
        // ends
        for( unsigned i = 0; i < ra.size(); ++i ) {
            Range &r = ra[ i ];
            for( int n = r.end + 1; ; ++n ) {
                if ( n == p_size or not not_in->p[ n ] ) {
                    if ( n != r.end + 1 ) { // extension is possible ?
                        // touching intervals ?
                        if ( i + 1 < ra.size() and ra[ i + 1 ].beg == n ) {
                            ra[ i ].end = ra[ i + 1 ].end;
                            ra.remove( i + 1 );
                            --i; // test again, with the new end
                            break;
                        }
                        // [ ... x - 1 ] | [ x + 1 ... ] if cardinal first interval > 1
                        if ( i + 1 < ra.size() and r.size() >= 2 and ra[ i + 1 ].beg == n + 1 ) {
                            ra[ i ].end = n - 1;
                            break;
                        }
                        // open interval
                        if ( n == p_size )
                            ra[ i ].end = p_size - 1;
                    }
                    break;
                }
            }
        }

        // begs
        for( unsigned i = 1; i < ra.size(); ++i ) {
            Range &r = ra[ i ];
            for( int n = r.beg - 1; ; --n ) {
                if ( n < 0 or not not_in->p[ n ] ) {
                    if ( n != r.beg - 1 ) { // extension is possible ?
                        // [ ... x - 1 ] | [ x + 1 ... ] if cardinal first interval > 1
                        if ( ra[ i - 1 ].size() >= 2 and ra[ i - 1 ].end == n - 1 ) {
                            ra[ i ].beg = n + 1;
                            break;
                        }
                        // open interval
                        if ( n < 0 )
                            ra[ i ].beg = 0;
                    }
                    break;
                }
            }
        }
    }

    // [ ... x - 1 ] | [ x + 1 ... ] if cardinal first interval > 1
    for( unsigned i = 1; i < ra.size(); ++i ) {
        if ( ra[ i - 1 ].size() >= 2 and ra[ i - 1 ].end + 2 == ra[ i ].beg ) {
            ra[ i - 1 ].exceptions << ra[ i - 1 ].end + 1;
            ra[ i - 1 ].end = ra[ i ].end;
            ra.remove( i-- );
        }
    }

    return ra;
}

Vec<Cond::Range> Cond::ko_ranges() const {
    Cond neg = ~ *this;
    return neg.ok_ranges();
}

String Cond::ok_cpp( String var, const Cond *not_in ) const {
    if ( always_checked( not_in ) )
        return "true";
    if ( never_checked( not_in ) )
        return "false";

    std::ostringstream os;
    Vec<Range> ra = ok_ranges_opt( not_in );
    for( unsigned i = 0; i < ra.size(); ++i ) {
        if ( i )
            os << " or ";
        ra[ i ].write_cpp( os, var, ra.size() > 1 );
    }
    return os.str();
}

String Cond::ko_cpp( String var, const Cond *not_in ) const {
    Cond tmp = ~ *this;
    return tmp.ok_cpp( var, not_in );
}

double Cond::cost_cpp( const Cond *not_in ) const {
    if ( always_checked( not_in ) )
        return 0;
    if ( never_checked( not_in ) )
        return 0;

    double res = 0;
    Vec<Range> ra = ok_ranges_opt( not_in );
    for( Range &r : ra )
        res += r.cost_cpp();
    return res;
}

int Cond::nz() const {
    return p.count();
    //    int res = 0;
    //    for( int i = 0; i < p_size; ++i )
    //        res += bool( p[ i ] );
    //    return res;
}

int Cond::first_nz() const {
    for( int i = 0; i < p_size; ++i )
        if ( p[ i ] )
            return i;
    return -1;
}

bool Cond::never_checked( const Cond *not_in ) const {
    if ( not_in ) {
        for( int i = 0; i < p_size; ++i )
            if ( not not_in->p[ i ] and p[ i ] )
                return false;
    } else {
        for( int i = 0; i < p_size; ++i )
            if ( p[ i ] )
                return false;
    }
    return true;
}

bool Cond::always_checked( const Cond *not_in ) const {
    if ( not_in ) {
        for( int i = 0; i < p_size; ++i )
            if ( not not_in->p[ i ] and not p[ i ] )
                return false;
        return true;
    }
    return p.all();
}

bool Cond::included_in( const Cond &cond ) const {
    for( int i = 0; i < p_size; ++i )
        if ( p[ i ] and not cond.p[ i ] )
            return false;
    return true;
}

void Cond::Range::write_to_stream( std::ostream &os ) const {
    int cpt = 1;
    if ( beg == end )
        cc( os, beg );
    else if ( beg and end < 255 )
        cc( cc( os, beg ) << "..", end );
    else if ( end < 255 )
        cc( os << "...", end );
    else if ( beg )
        cc( os, beg ) << "...";
    else if ( exceptions.empty() )
        os << "any";
    else
        cpt = 0;

    for( int v : exceptions )
        cc( os << ( cpt++ ? "&" : "" ) << "~", v );
}

void Cond::Range::write_cpp( std::ostream &os, const std::string &var, bool in_or ) const {
    bool par = in_or and beg != end and ( ( beg and end < 255 ) or exceptions.size() );
    if ( par )
        os << "( ";

    int cpt = 1;
    if ( beg == end )
        cc( os << var << " == ", beg );
    else if ( beg and end < 255 )
        cc( cc( os <<  var << " >= ", beg ) << " and " << var << " <= ", end );
    else if ( end < 255 )
        cc( os << var << " <= ", end );
    else if ( beg )
        cc( os << var << " >= ", beg );
    else if ( exceptions.empty() )
        os << "true";
    else
        cpt = 0;

    for( int v : exceptions )
        cc( os << ( cpt++ ? " and " : "" ) << var << " != ", v );

    if ( par )
        os << " )";
}

double Cond::Range::cost_cpp() const {
    double res = exceptions.size();

    if ( beg == end )
        return res + 1;
    if ( beg and end < 255 )
        return res + 2;
    if ( end < 255 )
        return res + 1;
    if ( beg )
        return res + 1;
    return res;
}

std::ostream &operator<<( std::ostream &os, const Cond &cond ) {
    Vec<Cond::Range> p = cond.ok_ranges_opt();
    for( unsigned i = 0; i < p.size(); ++i ) {
        if ( i )
            os << "|";
        os << p[ i ];
    }
    return os;
}

bool Cond::operator<( const Cond &c ) const {
    // TODO: optimize
    for( int i = 0; i < p_size; ++i ) {
        if ( p[ i ] > c.p[ i ] )
            return false;
        if ( p[ i ] < c.p[ i ] )
            return true;
    }
    return false;
}

} // namespace Hpipe

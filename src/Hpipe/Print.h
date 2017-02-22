#pragma once

#include "EnableIf.h"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace Hpipe {

/// classes with `write_to_stream` will be displayed by default with the corresponding methods
template<class T>
typename EnableIf<1,std::ostream,decltype(&T::write_to_stream)>::T &operator<<( std::ostream &os, const T &val ) {
    val.write_to_stream( os );
    return os;
}

/// classes with `begin` will be displayed by default with the corresponding method
template<class T>
typename EnableIf<1,std::ostream,decltype(&T::begin)>::T &operator<<( std::ostream &os, const T &val ) {
    int cpt = 0;
    for( const auto &v : val )
        os << ( cpt++ ? "," : "" ) << v;
    return os;
}

template<class T0,class T1>
std::ostream &operator<<( std::ostream &os, const std::pair<T0,T1> &val ) {
    return os << val.first << ":" << val.second;
}

template<class OS,class T0> void __my_print( OS &os, const T0 &t0 ) { os << t0 << std::endl; }
template<class OS,class T0,class... Args> void __my_print( OS &os, const T0 &t0, const Args &...args ) { os << t0 << ", "; __my_print( os, args... ); }

#ifndef PRINT
    #define PRINT( ... ) \
        Hpipe::__my_print( std::cout << #__VA_ARGS__ " -> ", __VA_ARGS__ );
    #define PRINTE( ... ) \
        Hpipe::__my_print( std::cerr << #__VA_ARGS__ " -> ", __VA_ARGS__ );
    #define PRINTN( ... ) \
        Hpipe::__my_print( std::cout << #__VA_ARGS__ " ->\n", __VA_ARGS__ );
    #define PRINTL( ... ) \
        Hpipe::__my_print( std::cout << __FILE__ << ":" << __LINE__ << ": " << #__VA_ARGS__ " -> ", __VA_ARGS__ );
    #define PRINTLE( ... ) \
        Hpipe::__my_print( std::cerr << __FILE__ << ":" << __LINE__ << ": " << #__VA_ARGS__ " -> ", __VA_ARGS__ );
    #define PRINTF( ... ) \
        Hpipe::__my_print( std::cout << __PRETTY_FUNCTION__ << ": " << #__VA_ARGS__ " -> ", __VA_ARGS__ );
#endif

#define PRINTRP( A ) \
    RO_CALL( write_to_stream, (A), std::cout << "  RP( " << #A << " ) -> " ); std::cout << std::endl

#define PRINTRPL( A ) \
    RO_CALL( write_to_stream, (A), std::cout << __FILE__ << ":" << __LINE__ << ": RP( " << #A << " ) -> " ); std::cout << std::endl

#define PRINTSTR( A ) \
    RO_CALL( write_structure, (A), std::cout << "  STR( " << #A << " ) -> " ); std::cout << std::endl

#define PRINTESTR( A ) \
    RO_CALL( write_structure, (A), std::cerr << "  STR( " << #A << " ) -> " ); std::cerr << std::endl

#define PRINTPD( A, DEVID ) \
    RO_CALL( write_patch_data, (A), std::cout << "  PD( " << #A << " ) -> ", 4, nullptr ); std::cout << std::endl

template<class T>
std::string to_string( const T &val ) {
    std::ostringstream ss;
    ss << val;
    return ss.str();
}

template<class T>
std::string to_string_hex( const T &val ) {
    std::ostringstream ss;
    ss << std::hex << val;
    return ss.str();
}

template<class T>
struct PointedValues {
    void write_to_stream( std::ostream &os ) const {
        int cpt = 0;
        for( const auto &v : val )
            os << ( cpt++ ? "," : "" ) << *v;
    }
    T val;
};

template<class T>
PointedValues<T> pointed_values( const T &val ) {
    return { val };
}

template<class T,class F>
struct MappedValues {
    void write_to_stream( std::ostream &os ) const {
        int cpt = 0;
        for( const auto &v : val )
            os << ( cpt++ ? "," : "" ) << fun( v );
    }
    T val;
    F fun;
};

template<class T,class F>
MappedValues<T,F> mapped_values( const T &val, const F &fun ) {
    return { val, fun };
}

template<class T>
struct Values {
    void write_to_stream( std::ostream &os ) const {
        int cpt = 0;
        for( const auto &v : val )
            os << ( cpt++ ? "," : "" ) << v;
    }
    const T &val;
};

template<class T>
Values<T> values( const T &val ) {
    return { val };
}

} // namespace Hpipe


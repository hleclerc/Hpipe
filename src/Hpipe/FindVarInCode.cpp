#include "FindVarInCode.h"

namespace Hpipe {

namespace {

bool can_be_in_varname( char c ) {
    return c == '_' or ( c >= '0' and c <= '9' ) or ( c >= 'a' and c <= 'z' ) or ( c >= 'A' and c <= 'Z' );
}

int count_slash( const std::string &str, std::string::size_type pos ) {
    int count = 0;
    for( ; pos--; ) {
        if ( str[ pos ] != '\\' )
            return count;
        ++count;
    }
    return count;
}

bool in_a_string( const std::string &str, std::string::size_type pos ) {
    int count = 0;
    for( ; pos--; ) {
        if ( str[ pos ] == '"' and count_slash( str, pos ) % 2 == 0 )
            ++count;
    }
    return count % 2 == 1;
}

bool in_a_multi_line_comment( const std::string &str, std::string::size_type pos ) {
    for( ; pos--; ) { // /* */
        if ( str[ pos ] == '/' and pos and str[ pos - 1 ] == '*' )
            return false;
        if ( str[ pos ] == '*' and pos and str[ pos - 1 ] == '/' )
            return true;
    }
    return false;
}

bool in_a_single_line_comment( const std::string &str, std::string::size_type pos ) {
    for( ; pos--; ) {
        if ( str[ pos ] == '\n' )
            return false;
        if ( str[ pos ] == '/' and pos and str[ pos - 1 ] == '/' )
            return true;
    }
    return false;
}

bool as_a_attribute( const std::string &str, std::string::size_type pos ) {
    for( ; pos--; ) {
        if ( str[ pos ] == '.' )
            return true;
        if ( str[ pos ] == ':' and pos and str[ pos - 1 ] == ':' )
            return true;
        if ( str[ pos ] == '>' and pos and str[ pos - 1 ] == '-' )
            return true;
        if ( str[ pos ] != '\n' and str[ pos ] != '\r' and str[ pos ] != '\t' and str[ pos ] != ' ' )
            return false;
    }
    return false;
}

}

std::string::size_type find_var_in_code( const std::string &str, const std::string &var, std::string::size_type pos ) {
    for( ; ; pos += var.size() ) {
        pos = str.find( var, pos );
        if ( pos == std::string::npos )
            return pos;
        // not the complete variable
        if ( pos and can_be_in_varname( str[ pos - 1 ] ) )
            continue;
        if ( pos + var.size() < str.size() and can_be_in_varname( str[ pos + var.size() ] ) )
            continue;
        // in a string
        if ( in_a_string( str, pos ) )
            continue;
        // in a comment
        if ( in_a_multi_line_comment( str, pos ) )
            continue;
        if ( in_a_single_line_comment( str, pos ) )
            continue;
        // attribute (., ->, ::)
        if ( as_a_attribute( str, pos ) )
            continue;
        return pos;
    }
}


} // namespace Hpipe

#include "Lexer.h"
#include <string.h>

namespace Hpipe {

// helper funcs
namespace {

static inline bool lower ( const char *a ) { return *a >= 'a' and *a <= 'z'; }
static inline bool upper ( const char *a ) { return *a >= 'A' and *a <= 'Z'; }
static inline bool number( const char *a ) { return *a >= '0' and *a <= '9'; }
static inline bool letter( const char *a ) { return lower ( a ) or upper ( a ); }
static inline bool begvar( const char *a ) { return letter( a ) or *a == '_'  ; }
static inline bool cntvar( const char *a ) { return begvar( a ) or number( a ); }

static const char *starts_with( const char *beg, const char *str ) {
    for( ; ; ++beg, ++str ) {
        if ( *str == 0 )
            return beg;
        if ( *beg != *str )
            return 0;
    }
}

}

Lexer::Lexer( ErrorList &error_list ) : first_item( Lexem::NONE, 0, 0, {} ), error_list( error_list ) {
    // static information on operators
    num_grp_bracket = -1;
    #define OPERATOR( S, N, G ) if ( strcmp( S, "[" ) == 0 ) num_grp_bracket = G;
    #include "Operators.h"
    #undef OPERATOR
}

Lexem *Lexer::read( Source *source ) {
    // init
    last = &first_item;
    while ( last->next )
        last = last->next;
    Lexem *res = last;

    for( int i = 0; i < max_op_grp; ++i )
        sibling[ i ] = 0;

    // tokenize
    const char *data = source->data;
    if ( not data ) {
        error_list.add( source, 0, ( "Impossible to open " + std::string( source->provenance ) ).c_str() );
        return res;
    }

    while ( int inc = read_tok( source, data ) )
        data += inc;

    // operators
    enum { need_none = 0, need_larg = 1, need_rarg = 2, need_barg = 3 };
    int behavior[ max_op_grp ];
    #define OPERATOR( S, N, G ) behavior[ G ] = N;
    #include "Operators.h"
    #undef OPERATOR

    for( int num_grp = max_op_grp - 1; num_grp >= 0; --num_grp ) {
        for( Lexem *item = sibling[ num_grp ]; item; item = item->sibling ) {
            switch( behavior[ num_grp ] ) {
            case need_none: break;
            case need_larg: assemble_larg( item ); break;
            case need_rarg: assemble_rarg( item ); break;
            case need_barg: assemble_barg( item ); break;
            }
        }
    }

    // assemble machines
    for( Lexem *item = res; item; item = item->next ) {
        if ( item->eq( Lexem::OPERATOR, "=" ) ) {
            for( Lexem *next = item->next; ; next = next->next ) {
                if ( not next or next->eq( Lexem::OPERATOR, "=" ) or next->type == Lexem::TEST or next->type == Lexem::TRAINING or next->type == Lexem::METHODS ) {
                    if ( next != item->next ) {
                        item->children[ 1 ] = item->next;
                        item->next->parent = item;
                        item->next->prev = 0;
                        if ( next ) {
                            next->prev->next = 0;
                            next->prev = item;
                        }
                        item->next = next;
                    }
                    break;
                }
            }
        }
    }

    return res->next;
}

const Lexem *Lexer::base() const {
    return &first_item;
}

const Lexem *Lexer::root() const {
    return first_item.next;
}

void Lexer::write_to_stream( std::ostream &os ) const {
    for( const Lexem *item = root(); item ; item = item->next )
        os << *item << "\n";
}

const Lexem *Lexer::find_machine( const Lexem *&args, const std::string &name ) const {
    // we're looking for the last one (that's why we don't break
    const Lexem *res = 0;
    for( const Lexem *item = root(); item ; item = item->next ) {
        if ( item->eq( Lexem::OPERATOR, "=" ) ) {
            const Lexem *c = item->children[ 0 ];
            if ( c->eq( Lexem::OPERATOR, "[" ) ) {
                if ( c->children[ 0 ]->eq( name ) ) {
                    args = c->children[ 1 ];
                    res = item->children[ 1 ];
                }
            } else if ( c->eq( name ) ) {
                args = 0;
                res = item->children[ 1 ];
            }
        }
    }
    return res;
}

int Lexer::read_tok( Source *source, const char *cur ) {
    const char *beg = cur;

    // variable
    if ( begvar( beg ) ) {
        while ( cntvar( ++cur ) );

        // beg_test ?
        if ( std::string( beg, cur ) == "beg_test" ) {
            cur += 8;
            while ( *cur and strncmp( cur, "\nend_test", 9 ) != 0 )
                ++cur;
            if ( *cur )
                cur += 9;
            return push_tok( source, beg, cur, Lexem::TEST );
        }

        // beg_training ?
        if ( std::string( beg, cur ) == "beg_training" ) {
            cur += 12;
            while ( *cur and strncmp( cur, "\nend_training", 13 ) != 0 )
                ++cur;
            if ( *cur )
                cur += 13;
            return push_tok( source, beg, cur, Lexem::TRAINING );
        }

        // beg_methods ?
        if ( std::string( beg, cur ) == "beg_methods" ) {
            cur += 11;
            while ( *cur and strncmp( cur, "\nend_methods", 12 ) != 0 )
                ++cur;
            return push_tok( source, beg + 11, cur + 1, Lexem::METHODS ) + 11 + 11;
        }

        return push_tok( source, beg, cur, Lexem::VARIABLE );
    }

    // number
    if ( number( beg ) ) {
        while ( number( ++cur ) );
        return push_tok( source, beg, cur, Lexem::NUMBER );
    }

    // "{"
    if ( *beg == '{' )
        return push_cpp( source, beg );

    // '...'
    if ( *beg == '\'' or *beg == '"' )
        return push_str( source, beg, *beg );

    // #
    if ( *beg == '#' ) {
        while ( *( ++cur ) and *cur != '\n' );
        return cur - beg;
    }

    // "("
    if ( *beg == '(' or *beg == ')' or *beg == '[' or *beg == ']' ) {
        return push_tok( source, beg, beg + 1, Lexem::OPERATOR, *beg == '[' ? num_grp_bracket : -1, *beg );
    }

    // space
    if ( *beg == ' ' or *beg == '\n' or *beg == '\r' or *beg == '\t' )
        return 1;

    // operators
    #define OPERATOR( S, N, G ) if ( const char *end = starts_with( beg, S ) ) return push_tok( source, beg, end, Lexem::OPERATOR, G );
    #include "Operators.h"
    #undef OPERATOR

    // ...
    return *beg != 0;
}

bool Lexer::err( const Lexem *l, const char *msg ) const {
    error_list.add( l ? l->source : 0, l ? l->beg : 0, msg );
    return false;
}

namespace {

bool only_spaces( const std::string &line ) {
    for( char c : line )
        if ( c != ' ' )
            return false;
    return true;
}

}

Vec<Lexer::TestData> Lexer::test_data() const {
    Vec<TestData> res;
    for( const Lexem *item = root(); item ; item = item->next ) {
        if ( item->type == Lexem::TEST ) {
            Lexer::TestData td;

            std::istringstream is( item->str.substr( 8, item->str.size() - 16 ) );
            std::getline( is, td.name );
            while ( td.name.size() and td.name[ 0 ] == ' ' )
                td.name = td.name.substr( 1 );

            enum { DM_none = 0, DM_input, DM_output, };
            int data_mode = DM_none;
            int nb_out_lines = 0;
            int nb_inp_lines = 0;
            std::string line;
            while ( std::getline( is, line ) ) {
                if ( line.size() >= 4 and line.substr( 0, 4 ) == "    " ) {
                    if ( line.size() >= 8 and line.substr( 4, 4 ) == "    " ) {
                        switch ( data_mode ) {
                        case 0:
                            err( item, "String inside test must have spaces at the beginning: 1 space for command names, at least 2 for command data" );
                            break;
                        case 1:
                            if ( nb_inp_lines++ )
                                td.inp += '\n';
                            td.inp += line.substr( 8 );
                            break;
                        case 2:
                            if ( nb_out_lines++ )
                                td.out += '\n';
                            td.out+= line.substr( 8 );
                            break;
                        }
                    } else if ( line.size() >= 7 and line.substr( 4, 3 ) == "inp" ) {
                        data_mode = 1;
                    } else if ( line.size() >= 7 and line.substr( 4, 3 ) == "out" ) {
                        data_mode = 2;
                    } else if ( not only_spaces( line ) ) {
                        data_mode = 0;
                        err( item, ( "Unknown test command '" + line + "'" ).c_str() );
                        break;
                    } else {
                        switch ( data_mode ) {
                        case 1: if ( nb_inp_lines++ ) td.inp += '\n'; break;
                        case 2: if ( nb_out_lines++ ) td.out += '\n'; break;
                        default: break;
                        }
                    }
                } else if ( not only_spaces( line ) ) {
                    err( item, "String inside test must have spaces at the beginning: 4 spaces for command names, at least 8 for command data" );
                } else {
                    switch ( data_mode ) {
                    case 1: if ( nb_inp_lines++ ) td.inp += '\n'; break;
                    case 2: if ( nb_out_lines++ ) td.out += '\n'; break;
                    default: break;
                    }
                }
            }

            res << td;
        }
    }
    return res;
}

Vec<Lexer::TrainingData> Lexer::training_data() const {
    Vec<TrainingData> res;
    for( const Lexem *item = root(); item ; item = item->next ) {
        if ( item->type == Lexem::TRAINING ) {
            Lexer::TrainingData td;
            std::string freq;

            std::istringstream is( item->str.substr( 12, item->str.size() - 24 ) );
            std::getline( is, td.name );
            while ( td.name.size() and td.name[ 0 ] == ' ' )
                td.name = td.name.substr( 1 );

            enum { DM_none = 0, DM_input, DM_freq, };
            int data_mode = DM_none;
            int nb_freq_lines = 0;
            int nb_inp_lines = 0;
            std::string line;
            while ( std::getline( is, line ) ) {
                if ( line.size() >= 4 and line.substr( 0, 4 ) == "    " ) {
                    if ( line.size() >= 8 and line.substr( 4, 4 ) == "    " ) {
                        switch ( data_mode ) {
                        case DM_none:
                            err( item, "String inside test must have spaces at the beginning: 1 space for command names, at least 2 for command data" );
                            break;
                        case DM_input:
                            if ( nb_inp_lines++ )
                                td.inp += '\n';
                            td.inp += line.substr( 8 );
                            break;
                        case DM_freq:
                            if ( nb_freq_lines++ )
                                freq += '\n';
                            freq += line.substr( 8 );
                            break;
                        }
                    } else if ( line.size() >= 7 and line.substr( 4, 3 ) == "inp" ) {
                        data_mode = DM_input;
                    } else if ( line.size() >= 8 and line.substr( 4, 4 ) == "freq" ) {
                        data_mode = DM_freq;
                    } else if ( not only_spaces( line ) ) {
                        data_mode = DM_none;
                        err( item, ( "Unknown test command '" + line + "'" ).c_str() );
                        break;
                    } else {
                        switch ( data_mode ) {
                        case DM_input: if ( nb_inp_lines++ ) td.inp += '\n'; break;
                        case DM_freq: if ( nb_freq_lines++ ) freq += '\n'; freq += line.substr( 8 ); break;
                        default: break;
                        }
                    }
                } else if ( not only_spaces( line ) ) {
                    err( item, "String inside test must have spaces at the beginning: 4 spaces for command names, at least 8 for command data" );
                } else {
                    switch ( data_mode ) {
                    case DM_input: if ( nb_inp_lines++ ) td.inp += '\n'; break;
                    case DM_freq: if ( nb_freq_lines++ ) freq += '\n'; freq += line.substr( 8 ); break;
                    default: break;
                    }
                }
            }

            size_t idx = 0;
            td.freq = std::stod( freq.c_str(), &idx );
            if ( not idx ) {
                if ( freq.size() )
                    err( item, "Freq does not appear to be a correct number" );
                td.freq = 1;
            }

            res << td;
        }
    }
    return res;
}

std::string Lexer::methods() const {
    std::string res;
    for( const Lexem *item = root(); item ; item = item->next ) {
        if ( item->type == Lexem::METHODS ) {
            if ( res.size() )
                res += '\n';
            res += item->str;
        }
    }
    return res;
}

int Lexer::push_cpp( Source *source, const char *cur ) {
    const char *beg = cur++;

    for( int nba = 1; *cur and nba; ++cur ) {
        if ( *cur == '}' ) { // }
            --nba;
        } else if ( *cur == '{' ) { // {
            ++nba;
        } else if ( cur[ 0 ] == '/' and cur[ 1 ] == '/' ) { // // ...
            for( ; *( ++cur ) and *cur != '\n'; );
        } else if ( cur[ 0 ] == '/' and cur[ 1 ] == '*' ) { // /* ... */
            for( cur += 2; *cur and ( cur[ 0 ] != '*' or cur[ 1 ] != '/' ); ++cur );
            cur += *cur != 0;
        } else if ( *cur == '"' ) { // "..."
            for( ; *( ++cur ) != '"' and *cur; cur += cur[ 0 ] == '\\' and cur[ 1 ] == '"' );
        } else if ( *cur == '\'' ) { // '.'
            cur += 2 + ( cur[ 1 ] == '\\' );
        }
    }

    return push_tok( source, beg, cur, Lexem::CODE );
}

int Lexer::push_str( Source *source, const char *cur, char lim ) {
    const char *beg = cur;
    while ( *( ++cur ) != lim ) {
        if ( not *cur ) {
            error_list.add( source, beg, "Unterminated string" );
            return 0;
        }
    }
    return push_tok( source, beg + 1, cur, Lexem::STRING ) + 2;
}

int Lexer::push_tok( Source *source, const char *beg, const char *end, Lexem::Type type, int grp, char delim ) {
    if ( delim == ')' or delim == ']' ) {
        if ( stack_last.empty() ) {
            error_list.add( last->source, beg, "Closing parenthesis without opening correspondance." );
            return end - beg;
        }
        last = stack_last.back();
        stack_last.pop_back();
        if ( ( delim == ')' and not last->eq( "(" ) ) or ( delim == ']' and not last->eq( "[" ) ) )
            err( last, "Non matching parenthesis." );
        return end - beg;
    }

    Lexem *item = lexem_pool.New( type, source, beg, std::string{ beg, end } );
    if ( last ) {
        last->next = item;
    } else {
        Lexem *p = stack_last.back();
        p->children[ p->eq( "[" ) ] = item;
        item->parent = p;
    }
    item->prev = last;
    last = item;

    if ( grp >= 0 ) {
        item->sibling = sibling[ grp ];
        sibling[ grp ] = item;
    } else
        item->sibling = 0;

    // PRINT( String( beg, end ), grp, type );
    if ( delim == '(' or delim == '[' ) {
        stack_last.push_back( item );
        last = 0;
    }

    return end - beg;
}

bool Lexer::assemble_rarg( Lexem *o ) {
    if ( not o->next )
        return err( o, "Operator needs a right expression." );
    if ( o->next->next )
        o->next->next->prev = o;

    o->children[ 0 ] = o->next;
    o->children[ 0 ]->parent = o;
    o->next = o->next->next;

    o->children[ 0 ]->prev = NULL;
    o->children[ 0 ]->next = NULL;
    return true;
}

bool Lexer::assemble_larg( Lexem *o ) {
    if ( not o->prev )
        return err( o, "Operator needs a left expression." );
    if ( o->prev->parent ) {
        o->parent = o->prev->parent;
        if (o->prev->parent->children[ 0 ] == o->prev)
            o->prev->parent->children[ 0 ] = o;
        else
            o->prev->parent->children[ 1 ] = o;
    }
    if ( o->prev->prev )
        o->prev->prev->next = o;

    o->children[ 0 ] = o->prev;
    o->children[ 0 ]->parent = o;
    o->prev = o->prev->prev;

    o->children[ 0 ]->next = NULL;
    o->children[ 0 ]->prev = NULL;
    return true;
}

bool Lexer::assemble_barg( Lexem *o, int need_left, int need_right ) {
    if ( need_right and not o->next )
        return err( o, "Operator needs a right expression." );
    if ( need_left and not o->prev )
        return err( o, "Operator needs a left expression." );
    // prev
    if ( o->prev->parent ) {
        o->parent = o->prev->parent;
        if (o->prev->parent->children[ 0 ] == o->prev)
            o->prev->parent->children[ 0 ] = o;
        else
            o->prev->parent->children[ 1 ] = o;
    }
    if ( o->prev->prev )
        o->prev->prev->next = o;

    o->children[ 0 ] = o->prev;
    o->children[ 0 ]->parent = o;
    o->prev = o->prev->prev;

    o->children[ 0 ]->next = NULL;
    o->children[ 0 ]->prev = NULL;

    // next
    if ( o->next->next )
        o->next->next->prev = o;

    o->children[ 1 ] = o->next;
    o->children[ 1 ]->parent = o;
    o->next = o->next->next;

    o->children[ 1 ]->prev = NULL;
    o->children[ 1 ]->next = NULL;

    return true;
}

void Lexer::TestData::write_to_stream( std::ostream &os ) const {
    os << "name=" << name << " inp=" << inp << " out=" << out;
}


} // namespace Hpipe

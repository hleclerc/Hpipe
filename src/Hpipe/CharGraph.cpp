#include "CharGraph.h"
#include "Assert.h"
#include "DotOut.h"
#include <algorithm>
#include <map>

namespace Hpipe {

namespace {

/// remove min nb spaces spaces at the left
std::string left_shifted( const std::string &str ) {
    std::string::size_type min_pos = 1e6;
    std::istringstream ss( str );
    std::string line;
    while( std::getline( ss, line ) ) {
        auto pos = line.find_first_not_of( ' ' );
        if ( pos != std::string::npos )
            min_pos = std::min( min_pos, pos );
    }
    if ( min_pos == 1e6 )
        return str;

    ss.clear();
    ss.seekg( 0 );
    std::string res;
    while( std::getline( ss, line ) ) {
        if ( line.find_first_not_of( ' ' ) != std::string::npos )
            res += ( res.size() ? "\n" : "" ) + line.substr( min_pos );
    }
    return res;
}

//void advance( CharItem *item ) {
//    CharItem *next = item->edges[ 0 ].item;
//    for( CharItem *prev : item->prev )
//        for( CharEdge &edge : prev->edges )
//            if ( edge.item == item )
//                edge.item = next;
//    item->edges = next->edges;
//    next->edges = CharEdge{ item };
//    next->prev  = item->prev;
//    item->prev  = next;
//}

}

CharGraph::CharGraph( Lexer &lexer ) : ok( true ), lexer( lexer ), base( CharItem::BEGIN ) {
    std::string method = lexer.methods();
    if ( ! method.empty() )
        methods.push_back_unique( method );
}

void CharGraph::read( const Lexem *lexem ) {

    // make a clone of the lexems, with subsituted variables
    Lexem *lexem_cl = clone( lexem, {} );
    // lexem_cl->display_dot();

    // transformation to CharItems (as a child of base)
    Vec<CharItem *> leaves;
    read( leaves, lexem_cl, &base );

    // resolve gotos
    for( const WaitGoto &wg : gotos ) {
        for( unsigned i = 0; ; ++i) {
            if ( i == labels.size() ) {
                lexer.err( 0, ( "Impossible to find the label " + wg.name ).c_str() );
                ok = false;
                break;
            }
            if ( labels[ i ].name == wg.name ) {
                for( CharItem *l : wg.inputs )
                    l->edges << labels[ i ].pivot;
                break;
            }
        }
    }

    // OK nodes
    char_item_ok = ci_pool.New( CharItem::OK );
    for( CharItem *leaf : leaves )
        leaf->edges << char_item_ok;

    // sort edges (priority comes first)
    apply( []( CharItem *item ) {
        std::sort( item->edges.begin(), item->edges.end(), []( const CharEdge &a, const CharEdge &b ) {
            return a.prio < b.prio;
        } );
    } );

    // remove pivots
    apply( []( CharItem *item ) {
        for( unsigned num_edge = 0; num_edge < item->edges.size(); ) {
            CharItem *next = item->edges[ num_edge ].item;
            if ( next->type == CharItem::PIVOT ) {
                HPIPE_ASSERT( next->edges.size() >= 1, "" );
                // replace item->edge[ { num_edge } ] by pivot->edges[ ... ]
                item->edges[ num_edge ].item = next->edges[ 0 ].item;
                for( unsigned ind = 1; ind < next->edges.size(); ++ind )
                    item->edges.insert( item->edges.begin() + num_edge + ind, 1, next->edges[ ind ].item );
            } else
                ++num_edge;
        }
    } );

    // remove non advancing cycles (like '()**')
    std::map<CharItem *,Vec<unsigned>> edges_to_remove;
    get_cycles( [&]( Vec<ItemNum> vi ) {
        for( unsigned i = 0; ; ++i ) {
            if ( i == vi.size() ) {
                edges_to_remove[ vi[ 0 ].item ] << vi[ 0 ].num;
                break;
            }
            if ( vi[ i ].item->advancer() )
                break;
        }
    } );

    for( const auto &p : edges_to_remove ) {
        for( unsigned i = p.second.size(); i--; ) {
            p.first->edges.erase( p.first->edges.begin() + p.second[ i ] );
            if ( p.first->edges.empty() )
                throw "Item is going nowhere";
        }
    }

    // update prev
    apply( []( CharItem *item ) {
        for( CharEdge &edge : item->edges )
            edge.item->prev << item;
    } );

    // foo_NEXT +1 -> +1 foo... (execute machine with less uncertainties)
    for( bool change = false; ; change = false ) {
        apply( [&]( CharItem *item ) {
            // END_STR_NEXT ( -> +1 )*
            if ( item->type == CharItem::END_STR_NEXT && item->next_are_with_prev_s_1( CharItem::NEXT_CHAR ) ) {
                item->type = CharItem::NEXT_CHAR;
                for( CharEdge &edge : item->edges ) {
                    edge.item->type = CharItem::END_STR;
                    edge.item->str = item->str;
                }
                change = true;
                return;
            }

            // END_STR_NEXT ( -> cond )*
            if ( item->type == CharItem::END_STR && item->next_are_with_prev_s_1( CharItem::COND ) ) {
                item->cond = item->edges[ 0 ].item->cond;
                item->type = CharItem::COND;
                for( CharEdge &edge : item->edges ) {
                    edge.item->type = CharItem::END_STR;
                    edge.item->str = item->str;
                }
                change = true;
                return;
            }
        } );
        if ( ! change )
            break;
    }
}

void CharGraph::read( Vec<CharItem *> &leaves, const Lexem *l, Vec<CharItem *> inputs ) {
    if ( not l ) {
        leaves.append( inputs );
        return;
    }

    if ( l->type == Lexem::VARIABLE ) {
        if ( l->eq( "eof" ) ) {
            CharItem *nxt = ci_pool.New( CharItem::_EOF );
            for( CharItem *input : inputs )
                input->edges << nxt;
            return read( leaves, l->next, nxt );
        }

        //
        HPIPE_ERROR( "should not happen at this point (variables are normally removed by clone)" );
    }

    if ( l->type == Lexem::STRING ) {
        for( char c : l->str ) {
            CharItem *nxt = ci_pool.New( CharItem::NEXT_CHAR );
            for( CharItem *input : inputs )
                input->edges << nxt;

            CharItem *std = ci_pool.New( Cond( c ) );
            nxt->edges << std;

            inputs = std;
        }
        return read( leaves, l->next, inputs );
    }

    if ( l->type == Lexem::NUMBER ) {
        CharItem *nxt = ci_pool.New( CharItem::NEXT_CHAR );
        for( CharItem *input : inputs )
            input->edges << nxt;

        CharItem *str = ci_pool.New( Cond( l->to_int() ) );
        nxt->edges << str;

        return read( leaves, l->next, str );
    }

    if ( l->type == Lexem::OPERATOR ) {
        if ( l->eq( "**" ) ) {
            // make a pivot item
            CharItem *p = ci_pool.New( CharItem::PIVOT );
            for( CharItem *input : inputs )
                input->edges << p;

            Vec<CharItem *> l_leaves;
            read( l_leaves, l->children[ 0 ], p );
            for( CharItem *leaf : l_leaves )
                leaf->edges << p;
            for( CharEdge &e : p->edges )
                e.prio = -1;

            return read( leaves, l->next, p );
        }

        if ( l->eq( "++" ) ) {
            // first instance
            Vec<CharItem *> i_leaves;
            read( i_leaves, l->children[ 0 ], inputs );

            // make a pivot item
            CharItem *p = ci_pool.New( CharItem::PIVOT );
            for( CharItem *input : i_leaves )
                input->edges << p;

            Vec<CharItem *> l_leaves;
            read( l_leaves, l->children[ 0 ], p );
            for( CharItem *leaf : l_leaves )
                leaf->edges << p;
            for( CharEdge &e : p->edges )
                e.prio = -1;

            return read( leaves, l->next, p );
        }

        if ( l->eq( "*" ) ) {
            // make a pivot item
            CharItem *p = ci_pool.New( CharItem::PIVOT );
            for( CharItem *input : inputs )
                input->edges << p;

            Vec<CharItem *> l_leaves;
            read( l_leaves, l->children[ 0 ], p );
            for( CharItem *leaf : l_leaves )
                leaf->edges << p;
            for( CharEdge &e : p->edges )
                e.prio = 1;
            read( leaves, l->next, p );

            return;
        }

        if ( l->eq( "+" ) ) {
            // first instance
            Vec<CharItem *> i_leaves;
            read( i_leaves, l->children[ 0 ], inputs );

            // make a pivot item
            CharItem *p = ci_pool.New( CharItem::PIVOT );
            for( CharItem *input : i_leaves )
                input->edges << p;

            Vec<CharItem *> l_leaves;
            read( l_leaves, l->children[ 0 ], p );
            for( CharItem *leaf : l_leaves )
                leaf->edges << p;
            for( CharEdge &e : p->edges )
                e.prio = 1;
            read( leaves, l->next, p );

            return;
        }

        if ( l->eq( "|" ) ) {
            // make a pivot item
            CharItem *p = ci_pool.New( CharItem::PIVOT );

            Vec<CharItem *> l_leaves;
            read( l_leaves, l->children[ 0 ], p );
            for( CharEdge &e : p->edges )
                e.prio = -1;

            read( l_leaves, l->children[ 1 ], p );

            // simplification for single_char | single_char
            Cond c0, c1;
            if ( p->edges.size() == 2 and get_cond( c0, p->edges[ 0 ].item ) and get_cond( c1, p->edges[ 1 ].item ) ) {
                CharItem *nxt = ci_pool.New( CharItem::NEXT_CHAR );
                for( CharItem *input : inputs )
                    input->edges << nxt;

                CharItem *str = ci_pool.New( c0 | c1 );
                nxt->edges << str;

                return read( leaves, l->next, str );
            }

            // else, register the pivot
            for( CharItem *input : inputs )
                input->edges << p;
            return read( leaves, l->next, l_leaves );
        }

        if ( l->eq( "??" ) ) {
            // make a pivot item
            CharItem *p = ci_pool.New( CharItem::PIVOT );
            for( CharItem *input : inputs )
                input->edges << p;

            Vec<CharItem *> l_leaves;
            read( l_leaves, l->children[ 0 ], p );
            for( CharEdge &e : p->edges )
                e.prio = -1;

            l_leaves << p;

            return read( leaves, l->next, l_leaves );
        }

        if ( l->eq( "?" ) ) {
            // make a pivot item
            CharItem *p = ci_pool.New( CharItem::PIVOT );
            for( CharItem *input : inputs )
                input->edges << p;

            Vec<CharItem *> l_leaves;
            read( l_leaves, l->children[ 0 ], p );
            for( CharEdge &e : p->edges )
                e.prio = 1;

            l_leaves << p;

            return read( leaves, l->next, l_leaves );
        }

        if ( l->eq( "(" ) ) {
            Vec<CharItem *> l_leaves;
            read( l_leaves, l->children[ 0 ], inputs );
            return read( leaves, l->next, l_leaves );
        }

        if ( l->eq( "[" ) ) {
            static std::pair<const char *,std::function<CharItem *(const std::string &)>> fs[] = {
                { "skip"        , [&]( const std::string &l ) { return ci_pool.New( CharItem::SKIP        , l ); } },
                { "add_str"     , [&]( const std::string &l ) { return ci_pool.New( CharItem::ADD_STR     , l ); } },
                { "clr_str"     , [&]( const std::string &l ) { return ci_pool.New( CharItem::CLR_STR     , l ); } },
                { "beg_str"     , [&]( const std::string &l ) { return ci_pool.New( CharItem::BEG_STR     , l ); } },
                { "beg_str_next", [&]( const std::string &l ) { return ci_pool.New( CharItem::BEG_STR_NEXT, l ); } },
                { "end_str"     , [&]( const std::string &l ) { return ci_pool.New( CharItem::END_STR     , l ); } },
                { "end_str_next", [&]( const std::string &l ) { return ci_pool.New( CharItem::END_STR_NEXT, l ); } },
            };

            for( const auto &p : fs ) {
                if ( l->children[ 0 ]->eq( p.first ) ) {
                    if ( l->children[ 1 ] and l->children[ 1 ]->next == nullptr and l->children[ 1 ]->str != "=" ) {
                        const Lexem *n = l->children[ 1 ];
                        if ( n->eq( Lexem::OPERATOR, "(" ) )
                            n = n->children[ 0 ];
                        CharItem *nxt = p.second( n->str );
                        for( CharItem *input : inputs )
                            input->edges << nxt;
                        return read( leaves, l->next, nxt );
                    }
                    lexer.err( l, ( std::string( p.first ) + " needs exactly 1 arg with exactly 1 value" ).c_str() );
                    ok = false;
                    return read( leaves, l->next, inputs );
                }
            }

            static std::pair<const char *,std::function<void(const std::string &)>> fv[] = {
                { "add_include"    , [&]( const std::string &l ) { includes     .push_back_unique( l );                 } },
                { "add_preliminary", [&]( const std::string &l ) { preliminaries.push_back_unique( left_shifted( l ) ); } },
                { "add_prel"       , [&]( const std::string &l ) { preliminaries.push_back_unique( left_shifted( l ) ); } },
            };

            for( const auto &p : fv ) {
                if ( l->children[ 0 ]->eq( p.first ) ) {
                    if ( l->children[ 1 ] and l->children[ 1 ]->next == nullptr and l->children[ 1 ]->str != "=" ) {
                        const Lexem *n = l->children[ 1 ];
                        if ( n->eq( Lexem::OPERATOR, "(" ) )
                            n = n->children[ 0 ];
                        p.second( n->str );
                    } else {
                        lexer.err( l, ( std::string( p.first ) + " needs exactly 1 arg with exactly 1 value" ).c_str() );
                        ok = false;
                    }
                    return read( leaves, l->next, inputs );
                }
            }

            if ( l->children[ 0 ]->eq( "add_variable" ) ) {
                Vec<Arg> cargs = get_cargs( l->children[ 1 ] );
                if ( cargs.size() < 2 ) {
                    lexer.err( l, "add_variable expects at least 2 arguments" );
                    ok = false;
                } else {
                    variables[ cargs[ 1 ].val->str ] = Variable{ cargs[ 0 ].val->str, cargs.size() >= 3 ? cargs[ 2 ].val->str : "" };
                }
                return read( leaves, l->next, inputs );
            }

            lexer.err( l, "Should be resolved by clone" );
            ok = false;
            return read( leaves, l->next, inputs );
        }

        if ( l->eq( ".." ) ) {
            int v_0 = l->children[ 0 ]->ascii_val();
            int v_1 = l->children[ 1 ]->ascii_val();
            if ( v_0 < 0 or v_1 < 0 ) {
                lexer.err( l, "'..' must be between two strings with only one char" );
                ok = false;
            }

            CharItem *nxt = ci_pool.New( CharItem::NEXT_CHAR );
            for( CharItem *input : inputs )
                input->edges << nxt;

            CharItem *str = ci_pool.New( Cond( std::min( v_0, v_1 ), std::max( v_0, v_1 ) ) );
            nxt->edges << str;

            return read( leaves, l->next, str );
        }

        if ( l->eq( "-" ) ) {
            CharItem tmp( CharItem::PIVOT );
            Vec<CharItem *> l_leaves;
            read( l_leaves, l->children[ 0 ], &tmp );
            read( l_leaves, l->children[ 1 ], &tmp );
            Cond a, b;
            if ( tmp.edges.size() != 2 or not get_cond( a, tmp.edges[ 0 ].item ) or not get_cond( b, tmp.edges[ 1 ].item ) ) {
                lexer.err( l, "'..' must be between two single char conditions" );
                ok = false;
            }

            CharItem *nxt = ci_pool.New( CharItem::NEXT_CHAR );
            for( CharItem *input : inputs )
                input->edges << nxt;

            CharItem *str = ci_pool.New( a & ~ b );
            nxt->edges << str;

            return read( leaves, l->next, str );
        }

        if ( l->eq( "if" ) ) {
            if ( l->children[ 0 ]->type != Lexem::CODE ) {
                lexer.err( l, "if currently only suport code ({...}) as parameter" );
                ok = false;
            }
            CharItem *nxt = ci_pool.New( CharItem::_IF, l->children[ 0 ]->str.substr( 1, l->children[ 0 ]->str.size() - 2 ) );
            for( CharItem *input : inputs )
                input->edges << nxt;
            return read( leaves, l->next, nxt );
        }

        if ( l->eq( "->" ) ) {
            gotos << WaitGoto{ l->children[ 0 ]->str, inputs };
            return;
        }

        if ( l->eq( "<-" ) ) {
            CharItem *nxt = ci_pool.New( CharItem::LABEL );
            for( CharItem *input : inputs )
                input->edges << nxt;
            labels << Label{ l->children[ 0 ]->str, nxt };

            return read( leaves, l->next, nxt );
        }

        ok = false;
        lexer.err( l, "Uknown operator type" );
        return;
    }

    if ( l->type == Lexem::CODE ) {
        CharItem *nxt = ci_pool.New( CharItem::CODE );
        nxt->str = l->str;

        for( CharItem *input : inputs )
            input->edges << nxt;
        return read( leaves, l->next, nxt );
    }

    if ( l->type == Lexem::TRAINING or l->type == Lexem::METHODS )
        return read( leaves, l->next, inputs );

    ok = false;
    lexer.err( l, "Unmanaged type" );
    HPIPE_ERROR( "type: %i", l->type );
}

int CharGraph::display_dot( const char *f, const char *prg ) const {
    std::ofstream os( f );

    os << "digraph LexemMaker {\n";
    ++CharItem::cur_op_id;
    base.write_dot_rec( os );
    os << "}\n";

    os.close();

    return exec_dot( f, prg );
}

void CharGraph::get_cycles(std::function<void(Vec<ItemNum>)> f) {
    ++CharItem::cur_op_id;
    get_cycles_rec( &base, f, {} );
}

void CharGraph::apply( std::function<void (CharItem *)> f ) {
    ++CharItem::cur_op_id;
    apply_rec( &base, f );
}

CharItem *CharGraph::root() {
    return &base;
}

namespace {

void get_next_ltor( Vec<const CharItem *> &nitems, const CharItem *item, bool impossible_ko ) {
    switch( item->type ) {
    case CharItem::COND:
    case CharItem::_EOF:
    case CharItem::_IF:
    case CharItem::OK:
    case CharItem::KO:
        nitems.push_back_unique( item );
        break;
    case CharItem::NEXT_CHAR:
        if ( ! impossible_ko )
            break;
    default:
        for( const CharEdge &e : item->edges )
            get_next_ltor( nitems, e.item, impossible_ko );
    }
}

bool leads_to_ok_rec( const Vec<const CharItem *> &items, std::set<Vec<const CharItem *> > &visited, bool impossible_ko ) {
    // dead end...
    if ( items.empty() )
        return false;

    // get next items that are COND, OK, KO, _IF, or _EOF, stopping if CharItem::NEXT
    Vec<const CharItem *> nitems;
    for( const CharItem *item : items )
        get_next_ltor( nitems, item, impossible_ko );

    // we have already seen this case ?
    std::sort( nitems.begin(), nitems.end() );
    if ( visited.count( nitems ) )
        return true;
    visited.insert( nitems );

    // directly a ok, a ko or an if ?
    for( const CharItem *item : nitems ) {
        if ( item->type == CharItem::OK )
            return true;
        if ( item->type == CharItem::KO or item->type == CharItem::_IF )
            return false;
    }

    // we (normally) have only conds and eof. => make the list of conds or eofs
    Cond covered;
    Vec<const CharItem *> eofs;
    for( const CharItem *item : nitems ) {
        if ( item->type == CharItem::_EOF ) {
            eofs << item;
            continue;
        }
        HPIPE_ASSERT( item->type == CharItem::COND, "..." );
        covered |= item->cond;
    }
    if ( not covered.always_checked() )
        return false; // possible to stop this path

    if ( eofs.size() && ! leads_to_ok_rec( eofs, visited, impossible_ko ) )
        return false;

    // get char sets
    Vec<Cond> conds;
    for( const CharItem *item : nitems ) {
        if ( item->type == CharItem::_EOF )
            continue;
        Vec<Cond> old_conds = conds;
        Cond t = item->cond;
        conds.clear();
        for( const Cond &c : old_conds ) {
            if ( Cond i = t & c )
                conds << i;
            if ( Cond i = ~t & c )
                conds << i;
            t &= ~c;
        }
        if ( t )
            conds << t;
    }

    // test that all the path lead to ok
    for( const Cond &c : conds ) {
        Vec<const CharItem *> citems;
        citems.reserve( nitems.size() );
        for( const CharItem *item : nitems )
            if ( item->type == CharItem::COND and ( c & item->cond ) )
                for( const CharEdge &e : item->edges )
                    citems.push_back_unique( e.item );
        if ( ! leads_to_ok_rec( citems, visited, impossible_ko ) )
            return false;
    }
    return true;
}

}

bool CharGraph::leads_to_ok( const Vec<const CharItem *> &items, bool impossible_ko ) {
    std::set<Vec<const CharItem *>> visited;
    return leads_to_ok_rec( items, visited, impossible_ko );
}

void CharGraph::err( const std::string &msg ) {
    lexer.err( nullptr, msg.c_str() );
}

void CharGraph::apply_rec( CharItem *item, std::function<void (CharItem *)> f ) {
    if ( item->op_id == CharItem::cur_op_id )
        return;
    item->op_id = CharItem::cur_op_id;

    f( item );

    for( const CharEdge &t : item->edges )
        apply_rec( t.item, f );
}

void CharGraph::get_cycles_rec( CharItem *item, std::function<void(Vec<ItemNum>)> f, Vec<ItemNum> vi ) {
    if ( item->op_id == CharItem::cur_op_id ) {
        if ( item->op_mp >= 0 )
            f( { vi.begin() + item->op_mp, vi.end() } );
        return;
    }
    item->op_id = CharItem::cur_op_id;
    item->op_mp = vi.size();
    vi << ItemNum{ item, 0 };

    for( unsigned num_edge = 0; num_edge < item->edges.size(); ++num_edge ) {
        vi.back().num = num_edge;
        get_cycles_rec( item->edges[ num_edge ].item, f, vi );
    }

    item->op_mp = -1;
}

bool CharGraph::get_cond( Cond &cond, CharItem *item ) {
    if ( item->type != CharItem::NEXT_CHAR or item->edges.size() != 1 or item->edges[ 0 ].item->type != CharItem::COND or item->edges[ 0 ].item->edges.size() or in_wait_goto( item->edges[ 0 ].item ) )
        return false;
    cond = item->edges[ 0 ].item->cond;
    return true;
}

Lexem *CharGraph::clone( const Lexem *&l, const Vec<Arg> &args, const char *stop ) {
    Lexem *beg = 0, *end = 0;
    clone( beg, end, l, args, stop );
    return beg;
}

void CharGraph::clone( Lexem *&beg, Lexem *&end, const Lexem *&l, const Vec<Arg> &args, const char *stop ) {
    for( ; l; l = l->next ) {
        if ( stop and l->eq( Lexem::OPERATOR, stop ) )
            break;

        // variables
        auto keyword = []( const std::string &str ) {
            return str == "eof";
        };
        if ( l->type == Lexem::VARIABLE and not keyword( l->str ) ) {
            clone( beg, end, l->str, l, {}, args );
            continue;
        }

        // function call ( func[ args ] )
        auto is_inline = []( const Lexem *l ) {
            return l->eq( "skip" ) or l->eq( "add_str" ) or l->eq( "clr_str" ) or l->eq( "beg_str" )  or l->eq( "beg_str_next" ) or l->eq( "end_str" )  or l->eq( "end_str_next" ) or
                   l->eq( "add_include" ) or l->eq( "add_prel" ) or l->eq( "add_preliminary" ) or l->eq( "add_variable" );
        };

        bool do_not_clone_ch_0 = false;
        if ( l->eq( Lexem::OPERATOR, "[" ) ) {
            if ( is_inline( l->children[ 0 ] ) ) {
                do_not_clone_ch_0 = true;
            } else {
                // get arguments
                Vec<Arg> cargs = get_cargs( l->children[ 1 ], args, true );

                // find machine
                if ( l->children[ 0 ]->type != Lexem::VARIABLE ) {
                    lexer.err( l->children[ 1 ], "xxx in xxx[] should be a variable" );
                    ok = false;
                }
                clone( beg, end, l->children[ 0 ]->str, l->children[ 0 ], cargs, args );
                continue;
            }
        }

        // default case
        Lexem *res = le_pool.New( l->type, l->source, l->beg, l->str );
        if ( beg ) end->next = res;
        else beg = res;
        end = res;

        // modifications
        switch ( l->type ) {
        case Lexem::STRING:
        case Lexem::CODE:
            for( const Arg &arg : args ) {
                const Lexem *val = arg.val;
                if ( val->eq( Lexem::OPERATOR, "(" ) )
                    val = val->children[ 0 ];
                if ( val->next ) {
                    lexer.err( val->next, "When a variable is used in a code, it is expected to be a single lexem" );
                    ok = false;
                }
                repl_all( res->str, arg.name, val->str );
            }
            break;
        default:
            break;
        }

        for( unsigned i = 0; i < 2; ++i ) {
            if ( const Lexem *ch = l->children[ i ] ) {
                if ( i == 0 and ( l->eq( Lexem::OPERATOR, "->" ) or l->eq( Lexem::OPERATOR, "<-" ) ) )
                    res->children[ i ] = le_pool.New( ch->type, ch->source, ch->beg, to_string( values( calls ) ) + " " + ch->str );
                else if ( i == 0 and do_not_clone_ch_0 )
                    res->children[ i ] = le_pool.New( ch->type, ch->source, ch->beg, ch->str );
                else
                    res->children[ i ] = clone( ch, args );
                res->children[ i ]->parent = res;
            }
        }
    }
}

void CharGraph::clone( Lexem *&beg, Lexem *&end, const std::string &name, const Lexem *l, Vec<Arg> cargs, const Vec<Arg> &args ) {
    calls.push_back( name );
    if ( calls.size() > 1000 ) {
        lexer.err( l, "call stack size exceeded" );
        ok = false;
        return;
    }

    // look in args
    for( const Arg &arg : args ) {
        if ( arg.name == name ) {
            if ( cargs.size() ) {
                lexer.err( l, "found an argument with the same name, but in this case, it should be used without arguments" );
                ok = false;
            }

            Lexem *res = le_pool.New( Lexem::OPERATOR, l->source, l->beg, "(" );
            if ( beg ) end->next = res;
            else beg = res;
            end = res;

            const Lexem *l = arg.val;
            res->children[ 0 ] = clone( l, {} );
            res->children[ 0 ]->parent = res;
            return calls.pop_back();
        }
    }


    // look in machines
    const Lexem *machine_args;
    if ( const Lexem *lex = lexer.find_machine( machine_args, name ) ) {
        // get the machine arguments and default values
        Vec<Arg> margs;
        for( ; machine_args; machine_args = machine_args->next ) {
            if ( machine_args->eq( "=" ) ) {
                Arg *arg = margs.new_elem();
                arg->name = machine_args->children[ 0 ]->str;
                arg->val  = clone( machine_args = machine_args->next, {}, "," );
                if ( not machine_args )
                    break;
            } else if ( not machine_args->eq( "," ) ) {
                Arg *arg = margs.new_elem();
                arg->name = machine_args->str;
            }
        }

        // assign name when not specified in cargs
        unsigned narg = 0;
        for( ; narg < cargs.size(); ++narg ) {
            if ( cargs[ narg ].name.size() )
                break;
            if ( narg >= margs.size() ) {
                lexer.err( l, "too much arguments" );
                ok = false;
                break;
            }
            cargs[ narg ].name = margs[ narg ].name;
        }

        // add arguments with default values (if not defined on cargs)
        for( ; narg < margs.size(); ++narg ) {
            bool already_specified = false;
            for( Arg &a : cargs ) {
                if ( a.name == margs[ narg ].name ) {
                    already_specified = true;
                    break;
                }
            }
            if ( not already_specified ) {
                if ( not margs[ narg ].val ) {
                    lexer.err( l, "not enough arguments" );
                    ok = false;
                    break;
                }
                cargs << margs[ narg ];
            }
        }

        Lexem *res = le_pool.New( Lexem::OPERATOR, l->source, l->beg, "(" );
        if ( beg ) end->next = res;
        else beg = res;
        end = res;

        res->children[ 0 ] = clone( lex, cargs );
        res->children[ 0 ]->parent = res;
        return calls.pop_back();
    }

    // predefined machine
    if ( name == "add_prel" ) {
        if ( cargs.size() != 1 ) { lexer.err( l, "add_prel expects exactly 1 argument" ); ok = false; }
        else preliminaries.push_back_unique( left_shifted( cargs[ 0 ].val->str ) );
        return calls.pop_back();
    }

//    if ( name == "add_variable" ) {
//        if ( cargs.size() < 2 ) { lexer.err( l, "variable expects 2 or 3 arguments (type, name, default value)" ); ok = false; }
//        else variables[ cargs[ 1 ].val->str ] = Variable{ cargs[ 0 ].val->str, cargs.size() >= 3 ? cargs[ 2 ].val->str : "" };
//        return calls.pop_back();
//    }

    lexer.err( l, "Impossible to find the corresponding machine" );
    ok = false;
    return calls.pop_back();
}

void CharGraph::repl_all( std::string &str, const std::string &src, const std::string &dst ) {
    for( std::string::size_type p = 0; ; ) {
        p = str.find( src, p );
        if ( p == std::string::npos )
            break;
        str = str.replace( p, src.size(), dst );
        p += dst.size();
    }
}

bool CharGraph::in_wait_goto( CharItem *item ) const {
    for( const WaitGoto &wg : gotos )
        for( const CharItem *ci : wg.inputs )
            if ( ci == item )
                return true;
    return false;
}

Vec<CharGraph::Arg> CharGraph::get_cargs( const Lexem *l, const Vec<Arg> &args, bool want_clone ) {
    Vec<Arg> res;
    for( const Lexem *item = l; item; item = item->next ) {
        if ( item->eq( "=" ) ) {
            Arg *arg = res.new_item();
            arg->name = item->children[ 0 ]->str;
            arg->val  = want_clone ? clone( item = item->next, args, "," ) : jump_after( item = item->next, "," );
        } else {
            Arg *arg = res.new_item();
            arg->val = want_clone ? clone( item, args, "," ) : jump_after( item, "," );
        }
        if ( not item )
            break;
    }
    return res;
}

const Lexem *CharGraph::jump_after( const Lexem *&l, const char *stop ) {
    const Lexem *res = l;
    while ( res->eq( "(" ) )
        res = res->children[ 0 ];
    do
        l = l->next;
    while ( l && ( stop == 0 || ! l->eq( stop ) ) );
    return res;
}

void CharGraph::Arg::write_to_stream( std::ostream &os ) const {
    if ( name.size() )
        os << name << "=";
    for( const Lexem *l = val; l; l = l->next )
        os << ( l == val ? "" : " " ) << *l;
}

} // namespace Hpipe

#include "CharGraph.h"
#include "Assert.h"
#include "DotOut.h"
#include <algorithm>

namespace Hpipe {

CharGraph::CharGraph( Lexer &lexer, const Lexem *lexem ) : lexer( lexer ), base( CharItem::BEGIN ) {
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

    // simplifications: remove pivots
    apply( []( CharItem *item ) {
        for( unsigned num_edge = 0; num_edge < item->edges.size(); ) {
            CharItem *next = item->edges[ num_edge ].item;
            if ( next->type == CharItem::PIVOT ) {
                ASSERT( next->edges.size() >= 1, "" );
                item->edges[ num_edge ].item = next->edges[ 0 ].item;
                for( unsigned ind = 1; ind < next->edges.size(); ++ind )
                    item->edges.insert( item->edges.begin() + num_edge + 1, 1, next->edges[ ind ].item );
            } else
                ++num_edge;
        }
    } );
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
        ERROR( "should not happen at this point (variables are normally removed by clone)" );
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
            auto single_char = []( const CharItem *item ) {
                return item->type == CharItem::COND and item->edges.size() == 0;
            };
            if ( p->edges.size() == 2 and single_char( p->edges[ 0 ].item ) and single_char( p->edges[ 1 ].item ) ) {
                CharItem *nxt = ci_pool.New( CharItem::NEXT_CHAR );
                for( CharItem *input : inputs )
                    input->edges << nxt;

                CharItem *str = ci_pool.New( p->edges[ 0 ].item->cond | p->edges[ 1 ].item->cond );
                nxt->edges << str;

                delete p;
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
                { "add_str", [&]( const std::string &l ) { return ci_pool.New( CharItem::ADD_STR, l ); } },
                { "clr_str", [&]( const std::string &l ) { return ci_pool.New( CharItem::CLR_STR, l ); } },
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
                    return read( leaves, l->next, inputs );
                }
            }

            static std::pair<const char *,std::function<void(const std::string &)>> fv[] = {
                { "add_include", [&]( const std::string &l ) { includes.insert( l ); } },
            };

            for( const auto &p : fv ) {
                if ( l->children[ 0 ]->eq( p.first ) ) {
                    if ( l->children[ 1 ] and l->children[ 1 ]->next == nullptr and l->children[ 1 ]->str != "=" ) {
                        const Lexem *n = l->children[ 1 ];
                        if ( n->eq( Lexem::OPERATOR, "(" ) )
                            n = n->children[ 0 ];
                        includes.insert( n->str );
                    } else
                        lexer.err( l, ( std::string( p.first ) + " needs exactly 1 arg with exactly 1 value" ).c_str() );
                    return read( leaves, l->next, inputs );
                }
            }

            ERROR( "Should be resolved by clone" );
        }

        if ( l->eq( ".." ) ) {
            int v_0 = l->children[ 0 ]->ascii_val();
            int v_1 = l->children[ 1 ]->ascii_val();
            if ( v_0 < 0 or v_1 < 0 )
                lexer.err( l, "'..' must be between two strings with only one char" );

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
         if ( tmp.edges.size() != 2 or not get_cond( a, tmp.edges[ 0 ].item ) or not get_cond( b, tmp.edges[ 1 ].item ) )
             lexer.err( l, "'..' must be between two single char conditions" );

         CharItem *nxt = ci_pool.New( CharItem::NEXT_CHAR );
         for( CharItem *input : inputs )
             input->edges << nxt;

         CharItem *str = ci_pool.New( a & ~ b );
         nxt->edges << str;

         return read( leaves, l->next, str );
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

        PRINT( *l );
        ERROR( "Uknown operator type" );
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

    ERROR( "Unmanaged type %i", l->type );
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

void CharGraph::apply( std::function<void (CharItem *)> f ) {
    ++CharItem::cur_op_id;
    apply_rec( &base, f );
}

CharItem *CharGraph::root() {
    return &base;
}

namespace {

void get_next_conds( Vec<const CharItem *> &nitems, const CharItem *item ) {
    if ( item->type == CharItem::COND or item->type == CharItem::_EOF or item->type == CharItem::OK or item->type == CharItem::KO )
        nitems.push_back_unique( item );
    else
        for( const CharEdge &e : item->edges )
            get_next_conds( nitems, e.item );
}

bool impossible_ko_rec( const Vec<const CharItem *> &items, std::set<Vec<const CharItem *> > &visited ) {
    // dead end...
    if ( items.empty() )
        return false;

    // get to conds or OK
    Vec<const CharItem *> nitems;
    for( const CharItem *item : items )
        get_next_conds( nitems, item );

    // ok/ko
    for( const CharItem *item : nitems ) {
        if ( item->type == CharItem::OK )
            return true;
        if ( item->type == CharItem::KO )
            return false;
    }

    // loop means OK: we're not going to find a KO this way
    std::sort( nitems.begin(), nitems.end() );
    if ( visited.count( nitems ) )
        return true;
    visited.insert( nitems );

    // we (normally) have only conds and eof. => make the list of conds
    Cond covered;
    Vec<const CharItem *> eofs;
    for( const CharItem *item : nitems ) {
        if ( item->type == CharItem::_EOF ) {
            eofs << item;
            continue;
        }
        ASSERT( item->type == CharItem::COND, "..." );
        covered |= item->cond;
    }
    if ( not covered.always_checked() )
        return false;

    if ( eofs.size() and not impossible_ko_rec( eofs, visited ) )
        return false;

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

    for( const Cond &c : conds ) {
        Vec<const CharItem *> citems;
        citems.reserve( nitems.size() );
        for( const CharItem *item : nitems )
            if ( item->type == CharItem::COND and ( c & item->cond ) )
                for( const CharEdge &e : item->edges )
                    citems.push_back_unique( e.item );
        if ( not impossible_ko_rec( citems, visited ) )
            return false;
    }

    //
    return true;
}

}

bool CharGraph::impossible_ko( const Vec<const CharItem *> &items ) {
    std::set<Vec<const CharItem *>> visited;
    return impossible_ko_rec( items, visited );
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

bool CharGraph::get_cond( Cond &cond, CharItem *item ) {
    if ( item->type != CharItem::NEXT_CHAR or item->edges.size() != 1 or item->edges[ 0 ].item->type != CharItem::COND )
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
            return l->eq( "add_str" ) or l->eq( "clr_str" ) or l->eq( "add_include" );
        };

        bool avoid_copy_ch_0 = false;
        if ( l->eq( Lexem::OPERATOR, "[" ) ) {
            if ( is_inline( l->children[ 0 ] ) )
                avoid_copy_ch_0 = true;
            else {
                // get arguments
                Vec<Arg> cargs;
                for( const Lexem *item = l->children[ 1 ]; item; item = item->next ) {
                    if ( item->eq( "=" ) ) {
                        Arg *arg = cargs.new_item();
                        arg->name = item->children[ 0 ]->str;
                        arg->val  = clone( item = item->next, args, "," );
                    } else {
                        Arg *arg = cargs.new_item();
                        arg->val = clone( item, args, "," );
                    }
                    if ( not item )
                        break;
                }

                // find machine
                if ( l->children[ 0 ]->type != Lexem::VARIABLE )
                    lexer.err( l->children[ 1 ], "xxx in xxx[] should ne a variable" );
                clone( beg, end, l->children[ 0 ]->str, l->children[ 0 ], cargs, args );
                continue;
            }
        } else if ( l->eq( Lexem::OPERATOR, "->" ) or l->eq( Lexem::OPERATOR, "<-" ) )
            avoid_copy_ch_0 = true;


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
                if ( val->next )
                    lexer.err( val->next, "When a variable is used in a code, it is expected to be a single lexem" );
                repl_all( res->str, arg.name, val->str );
            }
            break;
        default:
            break;
        }

        for( unsigned i = 0; i < 2; ++i) {
            if ( const Lexem *ch = l->children[ i ] ) {
                res->children[ i ] = avoid_copy_ch_0 and i == 0 ? le_pool.New( ch->type, ch->source, ch->beg, ch->str ) : clone( ch, args );
                res->children[ i ]->parent = res;
            }
        }
    }
}

void CharGraph::clone( Lexem *&beg, Lexem *&end, const std::string &name, const Lexem *l, Vec<Arg> cargs, const Vec<Arg> &args ) {
    calls.push( name );
    if ( calls.size() > 1000 ) {
        lexer.err( l, "call stack size exceeded" );
        return;
    }

    // look in args
    for( const Arg &arg : args ) {
        if ( arg.name == name ) {
            if ( cargs.size() )
                lexer.err( l, "found an argument with the same name, but in this case, it should be used without arguments" );

            Lexem *res = le_pool.New( Lexem::OPERATOR, l->source, l->beg, "(" );
            if ( beg ) end->next = res;
            else beg = res;
            end = res;

            const Lexem *l = arg.val;
            res->children[ 0 ] = clone( l, {} );
            res->children[ 0 ]->parent = res;
            return calls.pop();
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
        return calls.pop();
    }

    lexer.err( l, "Impossible to find the corresponding machine" );
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

void CharGraph::Arg::write_to_stream( std::ostream &os ) const {
    if ( name.size() )
        os << name << "=";
    for( const Lexem *l = val; l; l = l->next )
        os << ( l == val ? "" : " " ) << *l;
}

} // namespace Hpipe

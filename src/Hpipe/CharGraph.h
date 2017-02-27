#pragma once

#include "CharItem.h"
#include "Lexer.h"
#include "Pool.h"
#include "Vec.h"

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <stack>

namespace Hpipe {

/**
*/
class CharGraph {
public:
    struct ItemNum { CharItem *item; unsigned num; };

    CharGraph( Lexer &lexer, const Lexem *lexem );

    int                      display_dot  ( const char *f = ".char.dot", const char *prg = 0 ) const;
    void                     get_cycles   ( std::function<void(Vec<ItemNum>)> f );
    void                     apply        ( std::function<void(CharItem *)> f );
    CharItem                *root         ();

    static bool              leads_to_ok  ( const Vec<const CharItem *> &items, bool impossible_ko = false );

    Vec<Lexer::TrainingData> training_data() const { return lexer.training_data(); }

    std::string              methods      () const { return lexer.methods(); }

    void                     err          ( const std::string &msg );

    CharItem                *char_item_ok;
    std::set<std::string>    includes;
    Vec<std::string>         attributes;
    Vec<std::string>         preliminaries;

protected:
    struct Variable {
        std::string type;
    };
    struct Scope {
        Scope( int id, Scope *parent ) : id( id ), parent( parent ) {}
        using TM = std::unordered_map<std::string,Variable>;

        int                 id;
        Scope              *parent;
        TM                  variables;
    };
    struct WaitGoto {
        std::string         name;
        Vec<CharItem *>     inputs;
    };
    struct Label {
        std::string         name;
        CharItem           *pivot;
    };
    struct Arg {
        void write_to_stream( std::ostream &os ) const;

        std::string         name;
        const Lexem        *val;
    };

    void                    read          ( Vec<CharItem *> &leaves, const Lexem *l, Vec<CharItem *> inputs );
    void                    apply_rec     ( CharItem *item, std::function<void(CharItem *)> f );
    void                    get_cycles_rec( CharItem *item, std::function<void(Vec<ItemNum>)> f, Vec<ItemNum> vi );
    bool                    get_cond      ( Cond &cond, CharItem *item );
    Lexem                  *clone         ( const Lexem *&l, const Vec<Arg> &args, const char *stop = 0 );
    void                    clone         ( Lexem *&beg, Lexem *&end, const Lexem *&l, const Vec<Arg> &args, const char *stop = 0 );
    void                    clone         ( Lexem *&beg, Lexem *&end, const std::string &name, const Lexem *l, Vec<Arg> cargs, const Vec<Arg> &args );
    static void             repl_all      ( std::string &str, const std::string &src, const std::string &dst );
    bool                    in_wait_goto  ( CharItem *item ) const;

    Lexer                  &lexer;
    CharItem                base;
    int                     nb_scopes = 0;
    Vec<WaitGoto>           gotos;
    Vec<Label>              labels;
    Pool<Lexem>             le_pool;
    Pool<CharItem>          ci_pool;
    Vec<std::string>        calls;
};

} // namespace Hpipe

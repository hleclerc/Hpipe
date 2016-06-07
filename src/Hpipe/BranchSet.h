#pragma once

#include "StreamSep.h"
#include "AutoPtr.h"
#include "Print.h"
#include "Pool.h"
#include "Vec.h"

#include <functional>

namespace Hpipe {
class Instruction;

/**
*/
class BranchSet {
public:
    struct Range {
        void write_to_stream( std::ostream &os ) const;
        int  size           () const { return end - beg; }
        bool same_dst       ( const Range &that ) const;

        int           beg;
        int           end;

        Instruction  *inst;
        const char   *name;
        double        freq;
    };
    struct Node {
        Node( int beg, bool use_equ, Node *ok, Node *ko );
        Node( const Range &range );

        int           beg;
        bool          use_equ; ///< true means use ==; false means < (if not use_neg)
        bool          use_neg; ///< true and use_equ means !=; true and not use_equ means >=.

        AutoPtr<Node> ok;
        AutoPtr<Node> ko;

        Instruction  *inst;
        double        freq;
        const char   *name;

    };
    enum {
        COST_MISPREDICTION = 6,
        COST_TEST          = 1,
        COST_GOTO          = 1,
    };

    BranchSet( Vec<Range> ranges );

    void          write_to_stream( std::ostream &os ) const;

    AutoPtr<Node> root;
protected:
};

} // namespace Hpipe

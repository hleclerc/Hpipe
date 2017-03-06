#pragma once

#include "StreamSep.h"
#include "AutoPtr.h"
#include "Print.h"
#include "Pool.h"
#include "Vec.h"

#include <unordered_map>
#include <functional>

namespace Hpipe {
class Instruction;

/**
  Tool to make a MultiCond to graph of conds
*/
class BranchSet {
public:
    struct Range {
        void write_to_stream( std::ostream &os ) const;
        int  size           () const { return end - beg; }
        bool same_dst       ( const Range &that ) const;
        bool operator<      ( const Range &that ) const;
        bool operator==     ( const Range &that ) const;

        int           beg;
        int           end; ///< end is excluded

        Instruction  *inst;
        const char   *name;
        double        freq;
    };
    struct Node {
        Node( int beg, bool use_equ, Node *ok, Node *ko );
        Node( const Range &range );

        double mean_depth() const;

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
    double        mean_depth     () const; ///< weighted by freq

    AutoPtr<Node> root;

protected:
    struct HV { size_t operator()( const Vec<Range> &vr ) const {
        size_t res = vr.size();
        for( unsigned i = 0, o = 6; i < vr.size(); ++i, o += 7 ) {
            if ( o + 7 >= 8 * sizeof( size_t ) )
                o -= 8 * sizeof( size_t ) - 7;
            res ^= vr[ i ].beg << o;
        }
        return res;
    } };
    using HM = std::unordered_map<Vec<Range>,Node *,HV>;

    Node         *make_choice( const Vec<Range> &ranges );
    Node         *make_choice_syst( const Vec<Range> &ranges );
    // Node         *one_item_choice( const Vec<Range> &ranges );
    // double        cost_approx( const Vec<Range> &ranges, unsigned beg, unsigned end, double depth );
};

} // namespace Hpipe

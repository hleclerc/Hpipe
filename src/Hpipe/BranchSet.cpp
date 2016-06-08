#include "TypeConfig.h"
#include "BranchSet.h"
#include <algorithm>
#include <limits>

namespace Hpipe {

void BranchSet::Range::write_to_stream( std::ostream &os ) const {
    os << "[" << beg << "," << end << ",freq=" << freq << ",inst=" << inst << "]";
}

bool BranchSet::Range::same_dst( const BranchSet::Range &that ) const {
    return inst == that.inst and name == that.name;
}

void output_rec( std::ostream &ss, const BranchSet::Node *node ) {
    if ( node->ok ) {
        if ( node->use_equ ) {
            if ( node->use_neg ) {
                ss << "if ( *data != " << node->beg << " ) { ";
                output_rec( ss, node->ko.ptr() );
                ss << " } else { ";
                output_rec( ss, node->ok.ptr() );
                ss << " }";
            } else {
                ss << "if ( *data == " << node->beg << " ) { ";
                output_rec( ss, node->ok.ptr() );
                ss << " } else { ";
                output_rec( ss, node->ko.ptr() );
                ss << " }";
            }
        } else {
            if ( node->use_neg ) {
                ss << "if ( *data >= " << node->beg << " ) { ";
                output_rec( ss, node->ko.ptr() );
                ss << " } else { ";
                output_rec( ss, node->ok.ptr() );
                ss << " }";
            } else {
                ss << "if ( *data < " << node->beg << " ) { ";
                output_rec( ss, node->ok.ptr() );
                ss << " } else { ";
                output_rec( ss, node->ko.ptr() );
                ss << " }";
            }
        }
    } else {
        if ( node->name )
            ss << node->name;
        ss << "/*" << node->freq << "*/";
    }
}

void cost_and_freq( double &cost, BranchSet::Node *node ) {
    if ( node->ok ) {
        cost_and_freq( cost, node->ok.ptr() );
        cost_and_freq( cost, node->ko.ptr() );

        if ( node->ok->freq > node->ko->freq ) {
            cost += ( BranchSet::COST_TEST + BranchSet::COST_MISPREDICTION ) * node->ko->freq + BranchSet::COST_TEST * node->ok->freq;
            node->use_neg = true;
        } else {
            cost += ( BranchSet::COST_TEST + BranchSet::COST_MISPREDICTION ) * node->ok->freq + BranchSet::COST_TEST * node->ko->freq;
            node->use_neg = false;
        }

        node->freq = node->ok->freq + node->ko->freq;
    }
}

struct TreeChoiceItem {
    struct Item {
        void     write_to_stream( std::ostream &os ) const { os << num_in_range << ( use_equ ? "(==)" : "(<)" ); }

        unsigned num_in_range;
        bool     use_equ;
    };

    bool      next           () { queue.pop_back(); return not queue.empty(); }
    void      write_to_stream( std::ostream &os ) const;

    Vec<Item> queue;
};

void TreeChoiceItem::write_to_stream(std::ostream &os) const {
    os << "[";
    int cpt = 0;
    for( const Item &item : queue )
        os << ( cpt++ ? "," : "" ) << item;
    os << "]";
}

BranchSet::Node *get_bs_rec( Vec<TreeChoiceItem> &tci, unsigned &num_in_tci, const Vec<BranchSet::Range> &ranges, bool only_one_choice ) {
    // leaf
    if ( ranges.size() == 1 )
        return new BranchSet::Node{ ranges.back() };
    // need to describe the new possibilities ?
    if ( num_in_tci == tci.size() ) {
        TreeChoiceItem *t = tci.push_back();
        if ( only_one_choice ) {
            // TODO: provide a default better choice
            unsigned best_range = ( 1 + ranges.size() ) / 2;
            for( unsigned i = 0; i < ranges.size(); ++i )
                if ( ranges[ best_range ].freq < ranges[ i ].freq )
                    best_range = i;
            if ( best_range )
                t->queue.push_item( { best_range, ranges[ best_range ].size() == 1 } );
            else
                t->queue.push_item( { 1, false } );
        } else {
            // ==
            for( unsigned i = 1; i < ranges.size() - 1; ++i )
                if ( ranges[ i ].size() == 1 )
                    t->queue.push_item( { i, true } );
            // >=
            for( unsigned i = 1; i < ranges.size(); ++i )
                t->queue.push_item( { i, false } );
        }
    }
    //
    const TreeChoiceItem::Item &choice = tci[ num_in_tci++ ].queue.back();
    unsigned nr = choice.num_in_range;
    if ( choice.use_equ ) {
        Vec<BranchSet::Range> ko_ranges = ranges.without( nr );

        // ... A B A ... => ... A ...
        if ( nr and nr < ko_ranges.size() and ko_ranges[ nr - 1 ].same_dst( ko_ranges[ nr ] ) ) {
            ko_ranges[ nr - 1 ].freq += ko_ranges[ nr ].freq;
            ko_ranges[ nr - 1 ].end = ko_ranges[ nr ].end;
            ko_ranges.remove( nr );
        }

        return new BranchSet::Node( ranges[ nr ].beg, true,
            get_bs_rec( tci, num_in_tci, ranges[ nr ], only_one_choice ),
            get_bs_rec( tci, num_in_tci, ko_ranges   , only_one_choice )
        );
    }
    return new BranchSet::Node( ranges[ nr ].beg, false,
        get_bs_rec( tci, num_in_tci, ranges.up_to( nr ), only_one_choice ),
        get_bs_rec( tci, num_in_tci, ranges.from ( nr ), only_one_choice )
    );
}

BranchSet::BranchSet( Vec<Range> ranges ) {
    std::sort( ranges.begin(), ranges.end(), []( const Range &a, const Range &b ) {
        return a.beg < b.beg;
    } );

    // get the graph with the best cost
    double best_cost = std::numeric_limits<double>::max();
    // PRINT( ranges.size() );
    bool only_one_choice = ranges.size() >= 10;
    Vec<TreeChoiceItem> tci;
    while ( true ) {
        unsigned num_in_tci = 0;
        BranchSet::Node *trial = get_bs_rec( tci, num_in_tci, ranges, only_one_choice );

        double cost = 0;
        cost_and_freq( cost, trial );

        //                PRINT( tci );
        //                output_rec( std::cout << cost << ": ", trial );
        //                std::cout << "\n";

        if ( best_cost > cost ) {
            best_cost = cost;
            root = trial;
        } else
            delete trial;

        // next
        while ( not tci.empty() ) {
            if ( tci.back().next() )
                break;
            tci.pop_back();
        }
        if ( tci.empty() )
            break;
    }
}

void BranchSet::write_to_stream( std::ostream &os ) const {
    output_rec( os, root.ptr() );
}

BranchSet::Node::Node( int beg, bool use_equ, BranchSet::Node *ok, BranchSet::Node *ko ) : beg( beg ), use_equ( use_equ ), use_neg( false ), ok( ok ), ko( ko ) {
}

BranchSet::Node::Node(const Range &range ) {
    inst = range.inst;
    name = range.name;
    freq = range.freq;
}


} // namespace Hpipe

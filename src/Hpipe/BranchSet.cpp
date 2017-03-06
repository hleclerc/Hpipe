#include "TypeConfig.h"
#include "BranchSet.h"
#include <algorithm>
#include <limits>
#include <cmath>

namespace Hpipe {

namespace {

static void get_mean_depth_rec( double &sum, double &wgt, const BranchSet::Node *node, double depth ) {
    if ( node->ok )
        get_mean_depth_rec( sum, wgt, node->ok.ptr(), depth + 1 );
    if ( node->ko )
        get_mean_depth_rec( sum, wgt, node->ko.ptr(), depth + 1 );
    if ( ! node->ok && ! node->ko ) {
        sum += node->freq * depth;
        wgt += node->freq;
    }
}

static void output_rec( std::ostream &ss, const BranchSet::Node *node ) {
    if ( node->ok ) {
        bool use_neg = node->ok->freq < node->ko->freq;
        if ( node->use_equ ) {
            if ( use_neg ) {
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
            if ( use_neg ) {
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

static double cost_and_freq( BranchSet::Node *node, double cost ) {
    if ( node->ok ) {
        if ( node->ok->freq > node->ko->freq ) {
            return cost_and_freq( node->ok.ptr(), cost + BranchSet::COST_TEST + 0 * BranchSet::COST_MISPREDICTION ) +
                   cost_and_freq( node->ko.ptr(), cost + BranchSet::COST_TEST + 1 * BranchSet::COST_MISPREDICTION );
        }
        return cost_and_freq( node->ok.ptr(), cost + BranchSet::COST_TEST + 1 * BranchSet::COST_MISPREDICTION ) +
               cost_and_freq( node->ko.ptr(), cost + BranchSet::COST_TEST + 0 * BranchSet::COST_MISPREDICTION );
    }
    return node->freq * cost;
}

struct TreeChoiceItem {
    struct Item {
        void     write_to_stream( std::ostream &os ) const { os << num_in_range << ( use_equ ? "(==)" : "(<)" ); }

        unsigned num_in_range;
        bool     use_equ;
    };

    bool      next           () { queue.pop_back(); return not queue.empty(); }
    void      write_to_stream( std::ostream &os ) const { os << "["; int cpt = 0; for( const Item &item : queue ) os << ( cpt++ ? "," : "" ) << item; os << "]"; }

    Vec<Item> queue;
};

}

// ----------------------- Range -----------------------
void BranchSet::Range::write_to_stream( std::ostream &os ) const {
    os << "[" << beg << "," << end << ",freq=" << freq << ",inst=" << inst << "]";
}

bool BranchSet::Range::same_dst( const Range &that ) const {
    return inst == that.inst and name == that.name;
}

bool BranchSet::Range::operator<( const Range &that ) const {
    return std::make_tuple( this->beg, this->end ) < std::make_tuple( that.beg, that.end );
}

bool BranchSet::Range::operator==( const BranchSet::Range &that ) const {
    return std::make_tuple( this->beg, this->end ) == std::make_tuple( that.beg, that.end );
}

// ----------------------- Node -----------------------

BranchSet::Node *get_bs_rec( Vec<TreeChoiceItem> &tci, unsigned &num_in_tci, const Vec<BranchSet::Range> &ranges ) {
    // leaf
    if ( ranges.size() == 1 )
        return new BranchSet::Node{ ranges.back() };

    // need to describe the new possibilities ?
    if ( num_in_tci == tci.size() ) {
        TreeChoiceItem *t = tci.push_back();

        // ==
        for( unsigned i = 1; i < ranges.size() - 1; ++i )
            if ( ranges[ i ].size() == 1 )
                t->queue.push_item( { i, true } );
        // >=
        for( unsigned i = 1; i < ranges.size(); ++i )
            t->queue.push_item( { i, false } );
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
            get_bs_rec( tci, num_in_tci, ranges[ nr ] ),
            get_bs_rec( tci, num_in_tci, ko_ranges    )
        );
    }
    return new BranchSet::Node( ranges[ nr ].beg, false,
        get_bs_rec( tci, num_in_tci, ranges.up_to( nr ) ),
        get_bs_rec( tci, num_in_tci, ranges.from ( nr ) )
    );
}

// ----------------------- BranchSet -----------------------

BranchSet::BranchSet( Vec<Range> ranges ) {
    std::sort( ranges.begin(), ranges.end(), []( const Range &a, const Range &b ) {
        return a.beg < b.beg;
    } );

    root = make_choice( ranges );
    cost_and_freq( root.ptr(), 0 );
}

void BranchSet::write_to_stream( std::ostream &os ) const {
    output_rec( os, root.ptr() );
}

double BranchSet::mean_depth() const {
    return root->mean_depth();
}

BranchSet::Node *BranchSet::make_choice( const Vec<Range> &ranges ) {
    // if only one range
    if ( ranges.size() == 1 )
        return new Node( ranges.back() );

    //
    //    if ( ranges.size() < 10 )
    //        return make_choice_syst( ranges );

    // tot_freq
    double tot_freq = 0;
    for( unsigned i = 0; i < ranges.size(); ++i )
        tot_freq += ranges[ i ].freq;

    // else, find how to split with inequ (we want same freq * depth on the left than on the right)
    const auto approx_depth = []( unsigned l ) { return log2( 1 + l ); };
    double best_time = 1e300, cum_freq = 0;
    bool equ = false;
    unsigned best_cut = 0;
    for( unsigned i = 1; i < ranges.size(); ++i ) {
        cum_freq += ranges[ i - 1 ].freq;
        double fdl =              cum_freq   * approx_depth(                 i );
        double fdr = ( tot_freq - cum_freq ) * approx_depth( ranges.size() - i );
        double time = fdl + fdr;
        if ( best_time > time ) {
            best_time = time;
            best_cut = i;
        }
    }

    // try with equality
    for( unsigned i = 1; i < ranges.size() - 1; ++i ) {
        if ( ranges[ i ].size() == 1 ) {
            double fdl =              ranges[ i ].freq;
            double fdr = ( tot_freq - ranges[ i ].freq ) * approx_depth( ranges.size() );
            double time = fdl + fdr;
            if ( best_time > time ) {
                best_time = time;
                best_cut = i;
                equ = true;
            }
        }
    }


    if ( equ ) {
        Vec<Range> ko_ranges = ranges.without( best_cut );

        // contiguous ranges ( ... A A ... => ... A ... )
        if ( best_cut and best_cut < ko_ranges.size() and ko_ranges[ best_cut - 1 ].same_dst( ko_ranges[ best_cut ] ) ) {
            ko_ranges[ best_cut - 1 ].freq += ko_ranges[ best_cut ].freq;
            ko_ranges[ best_cut - 1 ].end = ko_ranges[ best_cut ].end;
            ko_ranges.remove( best_cut );
        }

        return new Node( ranges[ best_cut ].beg, true, make_choice( ranges[ best_cut ] ), make_choice( ko_ranges ) );
    }
    return new Node( ranges[ best_cut ].beg, false, make_choice( ranges.up_to( best_cut ) ), make_choice( ranges.from ( best_cut ) ) );
}

BranchSet::Node *BranchSet::make_choice_syst( const Vec<Range> &ranges ) {
    Node *res = 0;

    // get the graph with the best cost
    double best_cost = std::numeric_limits<double>::max();
    Vec<TreeChoiceItem> tci;
    while ( true ) {
        unsigned num_in_tci = 0;
        Node *trial = get_bs_rec( tci, num_in_tci, ranges );
        double cost = cost_and_freq( trial, 0 );

        if ( best_cost > cost ) {
            best_cost = cost;
            res = trial;
        } else
            delete trial;

        // next
        while ( not tci.empty() ) {
            if ( tci.back().next() )
                break;
            tci.pop_back();
        }
        if ( tci.empty() )
            return res;
    }
}


//BranchSet::Node *BranchSet::one_item_choice( const Vec<Range> &ranges ) {
//    // if only one range
//    if ( ranges.size() == 1 )
//        return new Node( ranges.back() );

//    // contiguous ranges ( ... A A ... => ... A ... )
//    for( unsigned i = 1; i < ranges.size(); ++i ) {
//        if ( ranges[ i ].same_dst( ranges[ i - 1 ] ) ) {
//            Vec<Range> n_ranges = ranges;
//            n_ranges[ i - 1 ].freq += n_ranges[ i ].freq;
//            n_ranges[ i - 1 ].end = n_ranges[ i ].end;
//            n_ranges.remove( i );
//            return one_item_choice( n_ranges );
//        }
//    }

//    // find the most frequent range
//    unsigned best_range = 0;
//    for( unsigned i = 1; i < ranges.size(); ++i )
//        if ( ranges[ best_range ].freq < ranges[ i ].freq )
//            best_range = i;
//    best_range += best_range == 0;

//    if ( ranges[ best_range ].size() == 1 )
//        return new Node( ranges[ best_range ].beg, true, one_item_choice( ranges[ best_range ] ), one_item_choice( ranges.without( best_range ) ) );
//    return new Node( ranges[ best_range ].beg, false, one_item_choice( ranges.up_to( best_range ) ), one_item_choice( ranges.from( best_range ) ) );
//}

//double BranchSet::cost_approx( const Vec<Range> &ranges, unsigned beg, unsigned end, double depth ) {
//    if ( end - beg == 1 )
//        return ranges[ beg ].freq * depth;

//    // test with inequality
//    double best_cost = 1e300;
//    for( unsigned i = beg + 1; i < end; ++i ) {
//        double cost = cost_approx( ranges, beg, i, depth + 1 ) +
//                      cost_approx( ranges, i, end, depth + 1 );
//        if ( best_cost > cost )
//            best_cost = cost;
//    }

//    // try with equality
//    for( unsigned i = beg + 1; i < end - 1; ++i ) {
//        if ( ranges[ i ].size() == 1 ) {
//            Vec<Range> nr; nr.reserve( end - beg - 1 );
//            nr.append( ranges.subvec( beg, i ) );
//            nr.append( ranges.subvec( i + 1, end ) );

//            double cost = cost_approx( ranges, i, i + 1, depth + 1 ) +
//                          cost_approx( nr, 0, nr.size(), depth + 1 );
//            if ( best_cost > cost )
//                best_cost = cost;
//        }
//    }

//    return best_cost;
//}


BranchSet::Node::Node( int beg, bool use_equ, Node *ok, Node *ko ) : beg( beg ), use_equ( use_equ ), ok( ok ), ko( ko ) {
    freq = ok->freq + ko->freq;
}

BranchSet::Node::Node( const Range &range ) {
    inst = range.inst;
    name = range.name;
    freq = range.freq;
}

double BranchSet::Node::mean_depth() const {
    double sum = 0, wgt = 0;
    get_mean_depth_rec( sum, wgt, this, 0 );
    return sum / wgt;
}


} // namespace Hpipe

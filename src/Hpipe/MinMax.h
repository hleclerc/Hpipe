#pragma once

#include <limits>
#include "Print.h"

namespace Hpipe {

template<class T>
struct MinMax {
    MinMax( T min = std::numeric_limits<T>::max(), T max = std::numeric_limits<T>::min() ) : min( min ), max( max ) {}

    MinMax &operator<<( T val ) {
        min = std::min( min, val );
        max = std::max( max, val );
        return *this;
    }

    MinMax &operator<<( MinMax val ) {
        min = std::min( min, val.min );
        max = std::max( max, val.max );
        return *this;
    }

    MinMax operator+( T val ) const {
        return { min + val, max + val };
    }

    void write_to_stream( std::ostream &os ) const {
        os << "[" << min << "," << max << "]";
    }

    T min, max;
};

} // namespace Hpipe


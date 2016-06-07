#pragma once

#include "Stream.h"
#include "Source.h"

namespace Hpipe {

/**
*/
class ErrorList {
public:
    ErrorList();

    void                         clear ();
    operator                     bool  () const; ///< true if no error
    void                         add   ( Source *source, const char *beg, const char *msg );

    StreamSepMaker out;
    bool                         display_escape_sequences;
    bool                         display_col;
    bool                         ok;
};

} // namespace Hpipe

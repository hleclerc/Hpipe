#pragma once

namespace Hpipe {

/**
*/
class Source {
public:
    Source( const char *filename, const char *text, bool need_cp = true );
    Source( const char *filename );
    ~Source();

    const char *provenance;
    const char *data;

    Source     *prev;
    char       *rese;
};

} // namespace Hpipe

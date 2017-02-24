#pragma once

namespace Hpipe {
class CharItem;

/**
*/
class CharEdge {
public:
    CharEdge( CharItem *item = 0, int prio = 0 ) : item( item ), prio( prio ) {}

    CharItem *item;
    int       prio;
};

} // namespace Hpipe

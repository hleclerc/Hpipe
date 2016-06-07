#pragma once

namespace Hpipe {
class CharItem;

/**
*/
class CharEdge {
public:
    CharEdge( CharItem *item, int prio = 0 ) : item( item ), prio( prio ) {}

    CharItem *item;
    int       prio;
};

} // namespace Hpipe

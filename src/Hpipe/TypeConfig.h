#pragma once

#include "EnableIf.h"
#include <stdint.h>

namespace Hpipe {
  
using Bool = bool;

using SI8  = int8_t;
using SI16 = int16_t;
using SI32 = int32_t;
using SI64 = int64_t;

using PI8  = uint8_t;
using PI16 = uint16_t;
using PI32 = uint32_t;
using PI64 = uint64_t;

using FP32 = float;
using FP64 = double;
using FP80 = long double;

using ST   = typename EnableIf<sizeof(void *)/4,SI32,SI64>::T;
using PT   = typename EnableIf<sizeof(void *)/4,PI32,PI64>::T;

struct IKnowWhatIDo {}; ///< used for dangerous calls
struct UseMove      {}; ///<
struct Nawak        {}; ///< a void structure that does not represent anything


}

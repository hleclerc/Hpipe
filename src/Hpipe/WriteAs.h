#pragma once

#include <utility>
#include "TypeList.h"

namespace Hpipe {

template<class Bq,class T>
void write_as( Bq bq, TypeList<T>, T &&val ) { bq << std::forward<T>( val ); }

template<class Bq,class T,class O>
void write_as( Bq bq, TypeList<T>, O &&val ) { bq << T( std::forward<O>( val ) ); }

} // namespace Hpipe





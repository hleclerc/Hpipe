#pragma once

namespace Hpipe {

template<class T>
inline const T *inc_ref( const T *p ) {
    ++p->cpt_use;
    return p;
}

template<class T>
inline T *inc_ref( T *p ) {
    ++p->cpt_use;
    return p;
}

template<class T>
inline void dec_ref( const T *ptr ) {
    if ( --ptr->cpt_use < 0 )
        delete ptr;
}

template<class T>
struct AutoDecRef {
    AutoDecRef( T *ptr ) : ptr( ptr ) {}
    ~AutoDecRef() { dec_ref( ptr ); }
    T *operator->() { return ptr; }
    T *ptr;
};

}

#pragma once

#include <stdlib.h>

namespace Hpipe {
  
struct Free {
    template<class T>
    void operator()( T *data ) { if ( data ) free( data ); }
};

struct DeleteArray {
    template<class T>
    void operator()( T *data ) { delete [] data; }
};

struct Delete {
    template<class T>
    void operator()( T *data ) { delete data; }
};

template<class T,class M=Delete>
struct AutoPtr {
    AutoPtr() : data( 0 ) {}
    AutoPtr( T *obj ) : data( obj ) {}
    AutoPtr( AutoPtr &&obj ) : data( obj.data ) { obj.data = 0; }
    AutoPtr( const AutoPtr &obj ) : data( obj.data ? new T( *obj.data ) : obj.data ) {}

    ~AutoPtr() {
        free_method( data );
    }

    AutoPtr &operator=( T *obj ) {
        if ( data )
            free_method( data );
        data = obj;
        return *this;
    }

    AutoPtr &reassign( T *obj ) {
        data = obj;
        return *this;
    }

    AutoPtr &operator=( const AutoPtr &obj ) {
        if ( data != obj.data ) {
            if ( data )
                free_method( data );
            data = obj.data ? new T( *obj.data ) : obj.data;
        }
        return *this;
    }

    void clear() {
        if ( data )
            free_method( data );
        data = 0;
    }

    operator bool() const { return data; }

    bool operator==( const T           *p ) const { return data == p;      }
    bool operator==( const AutoPtr<T>  &p ) const { return data == p.data; }

    const T *ptr() const { return data; }
    T *ptr() { return data; }

    const T *operator->() const { return data; }
    T *operator->() { return data; }
    const T &operator*() const { return *data; }
    T &operator*() { return *data; }

    template<class Os>
    void write_to_stream( Os &os ) const { if ( data ) os << data; else os << "NULL"; }

    mutable T *data;
    M free_method;
};

}

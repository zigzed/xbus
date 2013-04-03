/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef BUS_IOWORKER_H
#define BUS_IOWORKER_H
#include "binlog.h"
#include <list>

namespace bus {

    class file_base {
    public:
        static file_base* create();
        virtual ~file_base() {}
        virtual bool    open(const char* file, size_t cache) = 0;
        virtual off_t   seek(off_t pos, int whence) = 0;
        virtual bool    size(off_t size) = 0;
        virtual size_t  load(void* buf, size_t len) = 0;
        virtual size_t  save(const void* buf, size_t len) = 0;
        virtual bool    flush() = 0;
        virtual void    close() = 0;
    };

}

#endif

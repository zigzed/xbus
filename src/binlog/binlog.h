/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef XBUS_BINLOG_BINLOG_H
#define XBUS_BINLOG_BINLOG_H
#include "xbus/xbus.h"
#include "common/config.h"
#include <stdint.h>
#include <stddef.h>

namespace bus {

    typedef int64_t off_t;
    class file_base;

    class binlog {
    public:
        static void remove(const char* name);
        binlog(const char* name, off_t capacity);
        ~binlog();
        bool        save(xbus_msg* msg);
        xbus_msg*   load();
        bool        full();
        bool        empty();
    private:
        void        update();

        struct header {
            off_t   size;
            off_t   wpos;
            off_t   rpos;
        };

        enum { CACHE_SIZE = 2048 };
        enum { FLUSH_ITER = 1000 };
        header          header_;
        file_base*      hdrfp_;
        file_base*      putfp_;
        file_base*      getfp_;
        size_t          count_;
        char*           cache_;
    };

}

#endif

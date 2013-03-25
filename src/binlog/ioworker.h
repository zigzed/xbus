/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef BUS_IOWORKER_H
#define BUS_IOWORKER_H
#include "binlog.h"

namespace bus {

    /** io worker for sequence access */
    class seq_io_worker : public io_worker {
    public:
        seq_io_worker();
        void        serve(io_request *req);
        io_worker*  clone();
        const char* name() const;
    private:
        bool    aseek(fd_t fd, int64_t off);
        void    aread(io_request* req);
        void    awrite(io_request* req);

        int64_t wpos_;
        int64_t rpos_;
        fd_t    wfd_;
        fd_t    rfd_;
    };

    /** io worker for random access */
    class rnd_io_worker : public io_worker {

    };

}

#endif

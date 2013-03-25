/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef XBUS_BINLOG_BINLOG_H
#define XBUS_BINLOG_BINLOG_H
#include "xbus/xbus.h"
#include <stddef.h>
#include <vector>
#include <map>
#include "common/config.h"
#include "common/alg/channel.h"
#include "common/sys/threads.h"
namespace bus {

#ifdef  OS_WINDOWS
    typedef HANDLE  fd_t;
#else
    typedef int     fd_t;
#endif

    class io_request_cb;

    struct io_request {
        enum REQTYPE { READ, WRITE, STOP };
        fd_t    fid;
        REQTYPE req;
        size_t  len;
        int64_t off;
        void*   buf;
        void*   arg;
        io_request_cb* aio;
    };

    class io_request_cb  {
    public:
        virtual ~io_request_cb() {}
        virtual void on_finish(io_request* req) = 0;
        virtual void on_failed(io_request* req, int code) = 0;
    };

    class io_worker {
    public:
        virtual ~io_worker() {}
        virtual void        serve(io_request* req) = 0;
        virtual io_worker*  clone() = 0;
        virtual const char* name() const = 0;
    };

    class io_queues {
    public:
        explicit io_queues(io_worker* worker);
        void add_request(io_request* req);
        void start();
        void stop();
    private:
        typedef cxx::alg::channel<io_request, 256, cxx::sys::plainmutex >  channel_t;
        channel_t   queue_;
        io_worker*  works_;
    };

    class io_queues_group {
    public:
        explicit io_queues_group(io_worker* worker, size_t init_size);
        ~io_queues_group();
        void add_request(io_request* req);
        void start();
        void stop();
    private:
        io_queues* choose(io_request* req);

        typedef std::vector<io_queues* >        queue_t;
        typedef std::vector<cxx::sys::thread >  threads;
        queue_t     queues_;
        threads     thread_;
        io_worker*  worker_;
    };

    class binlog {
    public:
        binlog(io_queues_group* io_reactor, size_t buffer);
        xbus_msg*   front() const;
        void        pop_front();
        void        push_back(xbus_msg* msg);
    private:

    };

}

#endif

/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "ioworker.h"
#include "common/sys/error.h"
#ifdef  OS_WINDOWS
#else
#include <unistd.h>
#endif

namespace bus {

    seq_io_worker::seq_io_worker()
        : wpos_(0), rpos_(0), wfd_(-1), rfd_(-1)
    {
    }

    void seq_io_worker::serve(io_request *req)
    {
        if(req->req == io_request::READ) {
            if(rfd_ == -1) {
                rfd_ = req->fid;
            }
            if(rpos_ == 0) {
                rpos_ = req->off;
                if(!aseek(rfd_, rpos_)) {
                    int code = cxx::sys::err::get();
                    return req->aio->on_failed(req, code);
                }
            }

            ENFORCE(rpos_ == req->off)(rpos_)(req->off);
            ENFORCE(rfd_ == req->fid)(rfd_)(req->fid);

            aread(req);
        }
        else if(req->req == io_request::WRITE) {
            if(wfd_ == -1) {
                wfd_ = req->fid;
            }
            if(wpos_ == 0) {
                wpos_ = req->off;
                if(!aseek(wfd_, wpos_)) {
                    int code = cxx::sys::err::get();
                    return req->aio->on_failed(req, code);
                }
            }
            ENFORCE(wpos_ == req->off)(wpos_)(req->off);
            ENFORCE(wfd_ == req->fid)(wfd_)(req->fid);

            awrite(req);
        }
        else {
            req->aio->on_failed(req, -1);
        }
    }

    io_worker* seq_io_worker::clone()
    {
        seq_io_worker* w = new seq_io_worker();
        return w;
    }

    bool seq_io_worker::aseek(fd_t fd, int64_t off)
    {
#ifdef  OS_WINDOWS
#else
        int rc = ::lseek(fd, off, SEEK_SET);
        return rc >= 0;
#endif
    }

    void seq_io_worker::aread(io_request *req)
    {
#ifdef OS_WINDOWS
#else
        int rc = ::read(rfd_, req->buf, req->len);
        if(rc <= 0) {
            int code = cxx::sys::err::get();
            return req->aio->on_failed(req, code);
        }
#endif
        rpos_ += rc;
        if(rc < req->len) {
            memset((char* )req->buf + rc, 0, req->len - rc);
        }

        return req->aio->on_finish(req);
    }

    void seq_io_worker::awrite(io_request *req)
    {
#ifdef OS_WINDOWS
#else
        int rc = ::write(wfd_, req->buf, req->len);
        if(rc <= 0) {
            int code = cxx::sys::err::get();
            return req->aio->on_failed(req, code);
        }
#endif
        wpos_ += rc;

        return req->aio->on_finish(req);
    }

    const char* seq_io_worker::name() const
    {
        return "sequence io worker";
    }

}

/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "binlog.h"

namespace bus {

    io_queues::io_queues(io_worker *worker)
        : works_(worker)
    {
    }

    void io_queues::add_request(io_request *req)
    {
        queue_.send(*req);
    }

    void io_queues::start()
    {
        io_request req;
        while(true) {
            if(!queue_.recv(&req, 500))
                continue;

            if(req.req == io_request::STOP) {
                break;
            }

            works_->serve(&req);
        }
    }

    void io_queues::stop()
    {
        io_request req;
        req.req = io_request::STOP;
        add_request(&req);
    }

    io_queues_group::io_queues_group(io_worker *worker, size_t init_size)
        : worker_(worker)
    {
        if(init_size == 0)
            init_size = 1;

        for(size_t i = 0; i < init_size; ++i) {
            io_queues* q = new io_queues(worker_->clone());
            queues_.push_back(q);
        }
    }

    void io_queues_group::add_request(io_request *req)
    {
        io_queues* q = choose(req);
        q->add_request(req);
    }

    io_queues* io_queues_group::choose(io_request *req)
    {
        fd_t fd = req->fid;
        return queues_[fd % queues_.size()];
    }

    void io_queues_group::start()
    {
        for(size_t i = 0; i < queues_.size(); ++i) {
            io_queues* q = queues_[i];
            cxx::sys::thread t = cxx::sys::threadcontrol::create(cxx::MakeDelegate(q, &io_queues::start), "io_queues::start");
            thread_.push_back(t);
        }
    }

    void io_queues_group::stop()
    {
        for(size_t i = 0; i < queues_.size(); ++i) {
            queues_[i]->stop();
        }
        for(threads::iterator it = thread_.begin(); it != thread_.end(); ++it) {
            it->join();
        }
    }

    io_queues_group::~io_queues_group()
    {
        for(size_t i = 0; i < queues_.size(); ++i) {
            delete queues_[i];
        }
    }

}

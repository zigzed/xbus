/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "binlog.h"
#include "ioworker.h"

class io_callback : public bus::io_request_cb {
public:
    io_callback() : count_(0) {}
    void on_failed(bus::io_request *req, int code) {
        printf("failed: %d\n", code);
    }
    void on_finish(bus::io_request *req) {
        if(++count_ % 10000 == 0)
            printf("%d done\n", count_);
    }
private:
    int count_;
};

TEST(ioqueue, usage)
{
    bus::seq_io_worker worker;
    bus::io_queues  queue(&worker);

    bus::io_request request;
    io_callback     callback;
    request.fid = open("1.test", O_CREAT | O_RDWR, 0644);
    request.buf = malloc(512);
    request.len = 512;
    request.req = bus::io_request::WRITE;
    request.arg = 0;
    request.aio = &callback;

    cxx::sys::thread t = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&queue, &bus::io_queues::start), "test start");

    for(int i = 0; i < 1/*1000000*/; ++i) {
        queue.add_request(&request);
        request.off += 512;
    }
    queue.stop();
    t.join();

    close(request.fid);
    unlink("1.test");
}

TEST(ioqueue, group)
{
    bus::seq_io_worker      worker;
    bus::io_queues_group    queue(&worker, 2);

    bus::io_request req[2];
    io_callback     callback[2];
    req[0].fid = open("2.test", O_CREAT | O_RDWR, 0644);
    req[0].buf = malloc(512);
    req[0].len = 512;
    req[0].req = bus::io_request::WRITE;
    req[0].arg = 0;
    req[0].aio = &callback[0];
    req[1].fid = open("3.test", O_CREAT | O_RDWR, 0644);
    req[1].buf = malloc(512);
    req[1].len = 512;
    req[1].req = bus::io_request::WRITE;
    req[1].arg = 0;
    req[1].aio = &callback[1];

    queue.start();

    for(int i = 0; i < 1000000; ++i) {
        queue.add_request(&req[i % 2]);
        req[i % 2].off += 512;
    }
    queue.stop();

    close(req[0].fid);
    close(req[1].fid);
    //unlink("2.test");
    //unlink("3.test");
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}


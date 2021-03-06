/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/sys/cputimes.h"
#include "common/sys/threads.h"
#include "binlog.h"

TEST(binlog, write)
{
    bus::binlog blog("1.test", 32 * 1024 * 1024);
    ASSERT_EQ(blog.empty(), true);
    ASSERT_EQ(blog.full(), false);
    for(size_t i = 0; i < 1000000; ++i) {
        xbus_msg* msg = xbus_msg_init(0, &i, sizeof(i));
        blog.save(msg);
        xbus_msg_free(msg);
    }
}

TEST(binlog, read)
{
    bus::binlog blog("1.test", 32 * 1024 * 1024);

    ASSERT_EQ(blog.empty(), false);

    for(size_t i = 0; i < 1000000; ++i) {
        xbus_msg* msg = blog.load();
        if(msg) {
            void*   ptr = xbus_msg_buf(msg);
            int     len = xbus_msg_len(msg);
            ASSERT_EQ(len, sizeof(i));
            ASSERT_EQ(memcmp(ptr, &i, sizeof(i)), 0);
            xbus_msg_free(msg);
        }
    }
    ASSERT_EQ(blog.empty(), true);
    xbus_msg* msg = blog.load();
    ASSERT_EQ(msg == NULL, true);
}

TEST(binlog, checkpoint)
{
    bus::binlog blog("5.test", 32 * 1024 * 1024);

    for(size_t i = 0; i < 100000; ++i) {
        xbus_msg* msg = xbus_msg_init(i, &i, sizeof(i));
        blog.save(msg);
        xbus_msg_free(msg);
    }

    bus::binlog::checkpoint cp = blog.mounted();
    for(size_t i = 0; i < 50000; ++i) {
        xbus_msg* msg = blog.load();
        ASSERT_EQ(i, xbus_msg_tag(msg));
        ASSERT_EQ(sizeof(i), xbus_msg_len(msg));
        size_t buf;
        memcpy(&buf, xbus_msg_buf(msg), xbus_msg_len(msg));
        ASSERT_EQ(buf, i);
        xbus_msg_free(msg);
    }
    blog.restart(cp, bus::binlog::READER);

    for(size_t i = 0; i < 50000; ++i) {
        xbus_msg* msg = blog.load();
        ASSERT_EQ(i, xbus_msg_tag(msg));
        ASSERT_EQ(sizeof(i), xbus_msg_len(msg));
        size_t buf;
        memcpy(&buf, xbus_msg_buf(msg), xbus_msg_len(msg));
        ASSERT_EQ(buf, i);
        xbus_msg_free(msg);
    }

    cp = blog.mounted();
    for(size_t i = 0; i < 50000; ++i) {
        xbus_msg* msg = blog.load();
        ASSERT_EQ(i + 50000, xbus_msg_tag(msg));
        ASSERT_EQ(sizeof(i), xbus_msg_len(msg));
        size_t buf;
        memcpy(&buf, xbus_msg_buf(msg), xbus_msg_len(msg));
        ASSERT_EQ(buf, i + 50000);
        xbus_msg_free(msg);
    }

    blog.discard(cp);

    xbus_msg* msg = blog.load();
    ASSERT_EQ(msg == NULL, true);

    blog.remove("5.test");
}

TEST(binlog, writer_reader)
{
    bus::binlog blog("2.test", 1 * 1024 * 1024);

    srand(time(NULL));

    char temp[1024];
    bus::off_t total = 0;

    for(size_t i = 0; i < 1000000; ++i) {
        int tag = rand() % 1024 + 1;
        xbus_msg* msg1 = xbus_msg_init(tag, temp, tag);
        blog.save(msg1);
        total += (sizeof(tag) + sizeof(tag) + tag);

        xbus_msg* msg2 = blog.load();
        assert(msg2);
        if(xbus_msg_tag(msg2) != tag) {
            printf("error1: %d, %d\n", xbus_msg_tag(msg2), tag);
        }
        if(xbus_msg_len(msg2) != tag) {
            printf("error2: %d, %d\n", xbus_msg_len(msg2), tag);
        }

        xbus_msg_free(msg1);
        xbus_msg_free(msg2);
    }

    printf("total writen/read: %lld, %lld, %lld\n",
           total, blog.stat()->wpos, blog.stat()->wcnt);
}

//class BinLogThreadTest {
//public:
//    BinLogThreadTest() : log_(NULL) {}
//    void startup() {
//        log_ = new bus::binlog("3.test", 64 * 1024 * 1024);
//        srand(::time(NULL));
//    }
//    void cleanup() {
//        delete log_;
//    }

//    void writer() {
//        char temp[1024] = {0};
//        cxx::sys::cpu_times timer;
//        timer.start();
//        for(size_t i = 0; i < 1000000; ++i) {
//            int32_t tag = rand() % 1024;
//            ASSERT_LT(tag, 1024);
//            xbus_msg* msg = xbus_msg_init(tag, temp, tag);
//            while(!log_->save(msg)) {
//                cxx::sys::threadcontrol::sleep(1);
//            }
//            xbus_msg_free(msg);
//        }
//        timer.stop();
//        printf("writer: %s\n", timer.report().c_str());
//    }
//    void reader() {
//        cxx::sys::cpu_times timer;
//        timer.start();
//        for(size_t i = 0; i < 1000000; ++i) {
//            xbus_msg* msg = NULL;
//            while(!msg) {
//                msg = log_->load();
//                if(!msg)
//                    cxx::sys::threadcontrol::sleep(1);
//            }
//            int     len = xbus_msg_len(msg);
//            int     tag = xbus_msg_tag(msg);
//            ASSERT_EQ(tag, len);
//            ASSERT_LT(tag, 1024);
//            xbus_msg_free(msg);
//        }
//        timer.stop();
//        printf("reader: %s\n", timer.report().c_str());
//    }
//private:
//    bus::binlog*    log_;
//};

//TEST(binlog, writer_reader_thread)
//{
//    BinLogThreadTest    binlog;
//    binlog.startup();

//    cxx::sys::thread t1 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&binlog, &BinLogThreadTest::writer), "writer");
//    cxx::sys::thread t2 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&binlog, &BinLogThreadTest::reader), "reader");
//    t1.join();
//    t2.join();

//    binlog.cleanup();
//}

TEST(binlogset, simple)
{
    bus::binlogset blogs("test", 1 * 1024 * 1024, 3);
    ASSERT_EQ(blogs.size(), 3 * 1024 * 1024);
    ASSERT_EQ(blogs.used(), 0);
    ASSERT_FALSE(blogs.full());
    ASSERT_TRUE(blogs.empty());

    int temp = 3;
    xbus_msg* msg = xbus_msg_init(0, &temp, sizeof(temp));
    blogs.save(msg);
    xbus_msg_free(msg);

    msg = blogs.load();
    ASSERT_TRUE(msg != NULL);
    xbus_msg_free(msg);
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}


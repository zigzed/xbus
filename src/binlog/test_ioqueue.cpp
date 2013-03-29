/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/sys/cputimes.h"
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


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}


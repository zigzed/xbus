/** Copyright (C) wilburlang@gmail.com
 */
#include "binlog.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

static void usage();
static void stats(bus::binlog& blog);
static void print(xbus_msg* msg);

int main(int argc, char* argv[])
{
    int         opt;
    std::string file;
    bool        inspect_msg = false;
    while((opt = getopt(argc, argv, "hmf:")) != -1) {
        switch(opt) {
        case 'f':
            file = optarg;
            break;
        case 'm':
            inspect_msg = true;
            break;
        case 'h':
        default:
            usage();
            exit(1);
        }
    }

    if(file.empty()) {
        printf("expecting argument: -f <filename>\n");
        exit(3);
    }

    bus::binlog blog(file.c_str(), 0);
    stats(blog);

    if(inspect_msg) {
        bus::binlog::checkpoint cp = blog.mounted();
        xbus_msg* msg = NULL;
        do {
            msg = blog.load();
            if(msg) {
                print(msg);
                xbus_msg_free(msg);
            }
        } while(msg);

        blog.restart(cp, bus::binlog::READER);
    }

    return 0;
}

static void usage()
{
    printf("usage: binlog_iter -f <filename >\r\n");
}

static void stats(bus::binlog &blog)
{
    printf("binlog: size: %lld\n"
           "        used: %lld\n",
           blog.size(), blog.used());
}

static void print(xbus_msg *msg)
{
    printf("msg: %d %d\n", xbus_msg_len(msg), xbus_msg_tag(msg));
}

/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef BUS_PRODUCER_H
#define BUS_PRODUCER_H
#include "xbus/xbus.h"

namespace bus {

    class producer {
    public:
        producer(const char* addr, unsigned short, xbus_cfg* cfg) {}
        virtual ~producer() {}
        virtual int  push(const xbus_msg* msg) = 0;
        virtual void stop() = 0;
    };

    class producer_mgmt {
    public:
        explicit producer_mgmt(xbus_cfg* cfg);
        producer* get(const char* addr, unsigned short port);
    };

}

#endif

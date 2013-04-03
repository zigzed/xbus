/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef BUS_PRODUCER_H
#define BUS_PRODUCER_H
#include "xbus/xbus.h"

namespace bus {

    /** 一个主题对应一个producer，每个producer可能会被 attach() 多个 consumer 目标。
     * producer 需要将消息发送到每个 consumer。producer本身并不缓存任何消息。因此在
     * push() 操作后调用的 attach() 目标将收不到之前的消息
     */
    class producer {
    public:
        explicit producer(const char* topic);
        virtual void attach(const char* addr, unsigned short port, xbus_cfg* cfg) = 0;
        virtual void detach(const char* addr, unsigned short port, xbus_cfg* cfg) = 0;
        virtual int  push(const xbus_msg *msg) = 0;
        virtual void stop() = 0;
        virtual ~producer() {}
    };

    class producer_mgmt {
    public:
        explicit producer_mgmt(xbus_cfg* cfg);
        producer* get(const char* addr, unsigned short port);
    };

}

#endif

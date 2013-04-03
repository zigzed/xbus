/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef BUS_PRODUCER_H
#define BUS_PRODUCER_H
#include "xbus/xbus.h"

namespace bus {

    /** һ�������Ӧһ��producer��ÿ��producer���ܻᱻ attach() ��� consumer Ŀ�ꡣ
     * producer ��Ҫ����Ϣ���͵�ÿ�� consumer��producer�����������κ���Ϣ�������
     * push() ��������õ� attach() Ŀ�꽫�ղ���֮ǰ����Ϣ
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

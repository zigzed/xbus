/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "xbus_msg.h"
#include <string.h>
#include <stdlib.h>
#include <new>
#include "common/sys/atomic.h"

namespace bus {

    class xbus_msg_t : public xbus_msg {
    public:
        static xbus_msg_t* alloc(int32_t tag, int32_t len, const void* buf);
        static void        xfree(xbus_msg_t* msg);

        xbus_msg_t(int32_t tag, int32_t len);
    };


    xbus_msg_t* xbus_msg_t::alloc(int32_t tag, int32_t len, const void *buf)
    {
        int32_t     size = len + sizeof(xbus_msg_t) + sizeof(cxx::sys::atomic_t);
        char*       buff = (char* )malloc(size);
        xbus_msg_t* msg  = new (buff) xbus_msg_t(tag, len);
        msg->off = 0;
        msg->ref = new (buff + sizeof(xbus_msg_t)) cxx::sys::atomic_t(1);
        msg->buf = (char* )buff + sizeof(xbus_msg_t) + sizeof(cxx::sys::atomic_t);
        if(buf)
            memcpy(msg->buf, buf, len);
        return msg;
    }

    void xbus_msg_t::xfree(xbus_msg_t *msg)
    {
        if(--*(cxx::sys::atomic_t* )msg->ref == 0) {
            free(msg);
        }
    }

    xbus_msg_t::xbus_msg_t(int32_t t, int32_t l)
    {
        this->tag = t;
        this->len = l;
    }

}

#ifdef __cplusplus
extern "C" {
#endif

xbus_msg* xbus_msg_init(int tag, const void *buf, int len)
{
    return bus::xbus_msg_t::alloc(tag, len, buf);
}

void xbus_msg_free(xbus_msg *msg)
{
    bus::xbus_msg_t* m = (bus::xbus_msg_t* )msg;
    bus::xbus_msg_t::xfree(m);
}

int xbus_msg_copy(xbus_msg *dst, const xbus_msg *src)
{
    if(!dst || !src)
        return -1;

    if(dst == src)
        return 1;

    dst->len = src->len;
    dst->tag = src->tag;
    dst->off = src->off;
    dst->buf = src->buf;
    dst->ref = src->ref;
    ((cxx::sys::atomic_t* )src->ref)->add(1);

    return 1;
}

int xbus_msg_move(xbus_msg *dst, xbus_msg *src)
{
    if(!dst || !src)
        return -1;

    if(dst == src)
        return 1;

    dst->len = src->len;
    dst->tag = src->tag;
    dst->off = src->off;
    dst->buf = src->buf;
    dst->ref = src->ref;
    return 1;
}

void* xbus_msg_buf(const xbus_msg *msg)
{
    return msg->buf;
}

int xbus_msg_len(const xbus_msg *msg)
{
    return msg->len;
}

int xbus_msg_tag(const xbus_msg *msg)
{
    return msg->tag;
}

int64_t xbus_msg_off(const xbus_msg *msg)
{
    return msg->off;
}

#ifdef __cplusplus
}
#endif

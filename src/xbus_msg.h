/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef BUS_XBUS_MSG_H
#define BUS_XBUS_MSG_H
#include "xbus/xbus.h"

#ifdef __cplusplus
extern "C" {
#endif

struct xbus_msg {
    int32_t len;
    int32_t tag;
    int64_t off;
    void*   ref;
    void*   buf;
};

#ifdef __cplusplus
}
#endif

#endif

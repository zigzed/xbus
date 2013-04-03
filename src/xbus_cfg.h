/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef BUS_XBUS_CFG_H
#define BUS_XBUS_CFG_H
#include "xbus/xbus.h"

#ifdef __cplusplus
extern "C" {
#endif

struct xbus_cfg {
    enum {
        BUS_COMPRESS, BUS_FLUSH, BUS_THREAD,
        PUB_WINDOW, PUB_CACHE,
        SUB_PRIORITY, SUB_OFFSET,
        XBUS_CFG_SIZE
    };
    int64_t cfg[XBUS_CFG_SIZE];
};

#ifdef __cplusplus
}
#endif

#endif

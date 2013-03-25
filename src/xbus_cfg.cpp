/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "xbus_cfg.h"
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

XBUS_API xbus_cfg* xbus_cfg_init()
{
    xbus_cfg* cfg = (xbus_cfg* )malloc(sizeof(xbus_cfg));
}

XBUS_API void xbus_cfg_free(xbus_cfg* cfg)
{
    free(cfg);
}

XBUS_API int xbus_cfg_set(xbus_cfg* cfg, const char* config, int64_t value)
{
    if(!config)
        return -1;

    if(strcmp(config, "bus.compress")) {
        cfg->cfg[xbus_cfg::BUS_COMPRESS] = value;
    }
    else if(strcmp(config, "bus.flush")) {
        cfg->cfg[xbus_cfg::BUS_FLUSH] = value;
    }
    else if(strcmp(config, "bus.thread")) {
        cfg->cfg[xbus_cfg::BUS_THREAD] = value;
    }
    else if(strcmp(config, "pub.window")) {
        cfg->cfg[xbus_cfg::PUB_WINDOW] = value;
    }
    else if(strcmp(config, "sub.priority")) {
        cfg->cfg[xbus_cfg::SUB_PRIORITY] = value;
    }
    else if(strcmp(config, "sub.offset")) {
        cfg->cfg[xbus_cfg::SUB_OFFSET] = value;
    }
    else {
        return -2;
    }

    return 0;
}

#ifdef __cplusplus
}
#endif

/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef BUS_XBUS_H
#define BUS_XBUS_H

#if (defined(WIN32) || defined(WIN64)) && (defined(_MSC_VER) || defined(__MINGW32__))
    #if defined(DLL_EXPORTS)
        #define XBUS_API  __declspec(dllexport)
    #else
        #define XBUS_API  __declspec(dllimport)
    #endif
#else
    #define XBUS_API
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef void*   bus_handle;
    typedef void*   pro_handle;
    typedef void*   con_handle;

    struct xbus_cfg;
    struct xbus_msg;

    struct xbus_qos {
        enum TRX { AT_MOST_ONCE, AT_LEAST_ONCE, EXACTLY_ONCE };
        TRX delivery;           /**< default AT_MOST_ONCE */
        int lifetime;           /**< default 300 */
    };

    struct xbus_inf {
        char            topic[256];
        char            trx[64];
        char            rcv[64];
        xbus_qos::TRX   qos;
        xbus_inf*       next;
    };

    /** 创建一个 activebus 引擎节点
     * @param   cluster activebus cluster编号，有效值 [1024,65535)。不同 cluster
     * 的 activebus 节点之间无法通讯
     * @param   cfg     activebus配置参数
     * @return 返回创建的 activebus 引擎句柄。如果创建失败则返回 NULL
     */
    XBUS_API bus_handle xbus_create(int cluster, const xbus_cfg* cfg);

    /** 删除一个 activebus 引擎节点，并释放相关的资源
     * @param   bus     'bus_create' 创建的 activebus 引擎句柄
     * @return 如果成功则返回0,其他值为错误代码
     */
    XBUS_API int        xbus_remove(bus_handle bus);

    /** 获取或者创建一个 producer 对象，用于后续的 'xbus_push' 操作
     * @param   bus     'bus_create' 创建的 activebus 引擎句柄
     * @param   topic   producer 发送的消息的主题
     * @param   qos     producer 的服务质量
     * @return 返回一个 producer 句柄对象，用于后续的 'xbus_push' 操作。如果失败则返回 NULL
     */
    XBUS_API pro_handle xbus_get_producer(bus_handle bus, const char* topic, const xbus_qos* qos);

    /** 获取或者创建一个 consumer 对象，用于后续的 'xbus_pull' 操作
     * @param   bus     'bus_create' 创建的 activebus 引擎句柄
     * @param   topic   consumer 接收的消息主题，可以是一个正则表达式
     * @param   cfg     consumer 的配置。当前的配置项有：'sub_priority' 和 'sub_offset'。
     * @return 返回一个 producer 句柄对象，用于后续的 'xbus_pull' 操作。如果失败则返回 NULL
     */
    XBUS_API con_handle xbus_get_consumer(bus_handle bus, const char* topic, const xbus_cfg* cfg);

    /** 释放之前创建的 producer 对象 */
    XBUS_API int        xbus_del_producer(pro_handle pro);

    /** 释放之前创建的 consumer 对象 */
    XBUS_API int        xbus_del_consumer(con_handle con);

    /** 发送消息 */
    XBUS_API int        xbus_push(pro_handle pro, const xbus_msg* msg);

    /** 接收消息 */
    XBUS_API int        xbus_pull(con_handle con, xbus_msg* msg, int ms);

    /** 获取 activebus 当前的发送、接收的对象的信息 */
    XBUS_API xbus_inf*  xbus_info(bus_handle bus);

    XBUS_API xbus_msg*  xbus_msg_init(int tag, const void* buf, int len);
    XBUS_API void       xbus_msg_free(xbus_msg* msg);
    XBUS_API int        xbus_msg_copy(xbus_msg* dst, const xbus_msg *src);
    XBUS_API int        xbus_msg_move(xbus_msg* dst, xbus_msg* src);
    XBUS_API void*      xbus_msg_buf(const xbus_msg* msg);
    XBUS_API int        xbus_msg_len(const xbus_msg* msg);
    XBUS_API int        xbus_msg_tag(const xbus_msg* msg);

    XBUS_API xbus_cfg*  xbus_cfg_init();
    XBUS_API void       xbus_cfg_free(xbus_cfg* cfg);
    /** 设置 activebus 配置
     * @param   cfg     xbus_cfg 对象
     * @param   config  配置项名称，当前支持的配置项有：
     *  + bus.compress  默认为-1，不压缩
     *  + bus.flush     默认刷新周期为5分钟
     *  + bus.thread    默认线程数量为1
     *  + pub.window    默认发送滑动窗口大小为100
     *  + sub.priority  默认接收优先级为100
     *  + sub.offset    默认接收消息偏移量为0
     * @param   value   配置项的值
     * @return 如果设置成功则返回0。其他值表示错误
     */
    XBUS_API int        xbus_cfg_set(xbus_cfg* cfg, const char* config, int64_t value);

    /**
     * @example
    void test()
    {
        xbus_cfg* cfg = xbus_cfg_init();
        xbus_cfg_set(cfg, "bus.thread", 2);
        bus_handle bus = xbus_create(1024, cfg);

        pro_handle pro = xbus_get_producer(bus, "test", NULL);

        xbus_cfg_set(cfg, "sub.priority", 100);
        xbus_cfg_set(cfg, "sub.offset", 0);
        con_handle con = xbus_get_consumer(bus, "^t", cfg);
        xbus_cfg_free(cfg);

        int i = 0;
        for(i = 0; i < 100; ++i) {
            xbus_msg* msg = xbus_msg_init(0, &i, sizeof(i));
            xbus_push(pro, msg);
            xbus_msg_free(msg);
        }
        xbus_del_producer(pro);

        int j = 0;
        for(j = 0; j < i; ++j) {
            xbus_msg* msg = NULL;
            xbus_pull(con, msg, -1);
            assert(xbus_msg_len(msg) == sizeof(j));
            assert(*(int* )xbus_msg_buf(msg) == j);
            assert(xbus_msg_tag(msg) == 0);
            xbus_msg_free(msg);
        }
        xbus_del_consumer(con);

        xbus_remove(bus);
    }
    */



#ifdef __cplusplus
}
#endif

#endif

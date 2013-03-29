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

    /** ����һ�� activebus ����ڵ�
     * @param   cluster activebus cluster��ţ���Чֵ [1024,65535)����ͬ cluster
     * �� activebus �ڵ�֮���޷�ͨѶ
     * @param   cfg     activebus���ò���
     * @return ���ش����� activebus ���������������ʧ���򷵻� NULL
     */
    XBUS_API bus_handle xbus_create(int cluster, const xbus_cfg* cfg);

    /** ɾ��һ�� activebus ����ڵ㣬���ͷ���ص���Դ
     * @param   bus     'bus_create' ������ activebus ������
     * @return ����ɹ��򷵻�0,����ֵΪ�������
     */
    XBUS_API int        xbus_remove(bus_handle bus);

    /** ��ȡ���ߴ���һ�� producer �������ں����� 'xbus_push' ����
     * @param   bus     'bus_create' ������ activebus ������
     * @param   topic   producer ���͵���Ϣ������
     * @param   qos     producer �ķ�������
     * @return ����һ�� producer ����������ں����� 'xbus_push' ���������ʧ���򷵻� NULL
     */
    XBUS_API pro_handle xbus_get_producer(bus_handle bus, const char* topic, const xbus_qos* qos);

    /** ��ȡ���ߴ���һ�� consumer �������ں����� 'xbus_pull' ����
     * @param   bus     'bus_create' ������ activebus ������
     * @param   topic   consumer ���յ���Ϣ���⣬������һ��������ʽ
     * @param   cfg     consumer �����á���ǰ���������У�'sub_priority' �� 'sub_offset'��
     * @return ����һ�� producer ����������ں����� 'xbus_pull' ���������ʧ���򷵻� NULL
     */
    XBUS_API con_handle xbus_get_consumer(bus_handle bus, const char* topic, const xbus_cfg* cfg);

    /** �ͷ�֮ǰ������ producer ���� */
    XBUS_API int        xbus_del_producer(pro_handle pro);

    /** �ͷ�֮ǰ������ consumer ���� */
    XBUS_API int        xbus_del_consumer(con_handle con);

    /** ������Ϣ */
    XBUS_API int        xbus_push(pro_handle pro, const xbus_msg* msg);

    /** ������Ϣ */
    XBUS_API int        xbus_pull(con_handle con, xbus_msg* msg, int ms);

    /** ��ȡ activebus ��ǰ�ķ��͡����յĶ������Ϣ */
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
    /** ���� activebus ����
     * @param   cfg     xbus_cfg ����
     * @param   config  ���������ƣ���ǰ֧�ֵ��������У�
     *  + bus.compress  Ĭ��Ϊ-1����ѹ��
     *  + bus.flush     Ĭ��ˢ������Ϊ5����
     *  + bus.thread    Ĭ���߳�����Ϊ1
     *  + pub.window    Ĭ�Ϸ��ͻ������ڴ�СΪ100
     *  + sub.priority  Ĭ�Ͻ������ȼ�Ϊ100
     *  + sub.offset    Ĭ�Ͻ�����Ϣƫ����Ϊ0
     * @param   value   �������ֵ
     * @return ������óɹ��򷵻�0������ֵ��ʾ����
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

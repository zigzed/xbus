/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef XBUS_BINLOG_BINLOG_H
#define XBUS_BINLOG_BINLOG_H
#include "xbus/xbus.h"
#include "common/config.h"
#include <stdint.h>
#include <stddef.h>
#include <string>
#include <deque>

namespace bus {

    typedef int64_t off_t;
    class file_base;

    class binlog {
    public:
        struct header {
            off_t   size;
            off_t   wpos;
            off_t   rpos;
            off_t   wcnt;
            off_t   rcnt;
        };
        typedef void*   checkpoint;
        enum CP_TYPE { READER = 1, WRITER = 2, BOTH = READER | WRITER };

        static void remove(const char* name);
        binlog(const char* name, off_t capacity, size_t cache = 2 * 1024 * 1024);
        ~binlog();
        bool        save(xbus_msg* msg);
        xbus_msg*   load();
        bool        full() const;
        off_t       used() const;
        off_t       size() const;
        const header* stat() const;
        bool        empty() const;
        void        remove();

        checkpoint  mounted();
        bool        restart(checkpoint cp, CP_TYPE type);
        void        discard(checkpoint cp);
    private:
        void        update();

        enum { CACHE_SIZE = 2048 };
        enum { FLUSH_ITER = 1000 };
        header          header_;
        file_base*      hdrfp_;
        file_base*      putfp_;
        file_base*      getfp_;
        size_t          count_;
        char*           cache_;
        std::string     fname_;
    };

    class binlogset {
    public:
        binlogset(const char* dir, off_t size, size_t count);
        ~binlogset();
        bool        save(xbus_msg* msg);
        xbus_msg*   load();
        bool        full() const;
        off_t       used() const;
        off_t       size() const;
        bool        empty() const;
    private:
        void        scan_dir();
        binlog*     make_log();
        void        clean_up();

        typedef std::deque<binlog* >    binlogset_t;
        binlogset_t logs_;
        off_t       size_;
        size_t      count_;
        size_t      maxid_;
        std::string file_;
        binlog*     writer_;
        binlog*     reader_;
    };

    /** binlog_mgmt 管理主题的创建，同时会在binlog满的时候自动创建新的文件，读取时自动
     * 轮换 binlog 文件，删除已经空的binlog文件。如果某个主题的条数或者时间超过一定的
     * 阀值，也会删除该 binlog 文件。
     */
    class binlog_mgmt {
    public:
        explicit binlog_mgmt(const char* base_dir);

        bool        put(const std::string& topic, xbus_msg* msg);
        xbus_msg*   get(std::string& topic);
    private:
        // 加载目录下的 binlog
        void    loaddir(const char* dir);
        // 清理目录下的 binlog
        void    cleanup(const char* dir);
    };

}

#endif

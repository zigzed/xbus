/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "binlog.h"
#include "xbus/xbus.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ioworker.h"
#include "../xbus_msg.h"

namespace bus {

    static bool     is_file_exist(const char* name);
    static size_t   ceil(size_t orig, size_t align)
    {
        return (orig + align - 1) / align * align;
    }


    ////////////////////////////////////////////////////////////////////////////
    binlog::binlog(const char *name, off_t capacity, size_t cache)
        : hdrfp_(NULL), putfp_(NULL), getfp_(NULL), count_(0), cache_(NULL),
          fname_(name)
    {
        bool existed = is_file_exist(name);
        hdrfp_ = file_base::create();
        putfp_ = file_base::create();
        getfp_ = file_base::create();
        hdrfp_->open(name, 4096);
        putfp_->open(name, cache);
        getfp_->open(name, cache);

        if(existed) {
            hdrfp_->seek(0, SEEK_SET);
            hdrfp_->load(&header_, sizeof(header_));
        }
        else {
            header_.size = ceil(capacity, sizeof(int32_t));
            header_.wpos = 0;
            header_.rpos = 0;
            header_.wcnt = 0;
            header_.rcnt = 0;
            hdrfp_->size(sizeof(header) + header_.size);
            off_t off = hdrfp_->seek(0, SEEK_SET);
            assert(off == 0);
            hdrfp_->save(&header_, sizeof(header_));
            hdrfp_->flush();
        }
        putfp_->seek(sizeof(header_) + header_.wpos % header_.size, SEEK_SET);
        getfp_->seek(sizeof(header_) + header_.rpos % header_.size, SEEK_SET);
        cache_ = new char[CACHE_SIZE];
    }

    binlog::~binlog()
    {
        if(hdrfp_) {
            hdrfp_->seek(0, SEEK_SET);
            hdrfp_->save(&header_, sizeof(header_));
            hdrfp_->close();
            delete hdrfp_;
        }

        if(putfp_) {
            putfp_->close();
            delete putfp_;
        }

        if(getfp_) {
            getfp_->close();
            delete getfp_;
        }

        delete[] cache_;
    }

    /** 磁盘存储结构 (4 + 4 + n）
     *  LENGTH  4 bytes, 数据 buffer 的长度，不包括LEN, TAG
     *  TAG     4 bytes,
     *  BUFFER  n bytes, n = LENGTH
     *  PADING  0~3 bytes
     */
    bool binlog::save(xbus_msg *msg)
    {
        size_t totals = xbus_msg_len(msg) + sizeof(int32_t) + sizeof(int32_t);
        size_t needed = ceil(totals, sizeof(int32_t));
        size_t padding= needed - totals;

        if(header_.size + header_.rpos < header_.wpos + needed)
            return false;

        off_t wpos = header_.wpos % header_.size;
        off_t rpos = header_.rpos % header_.size;
        // 如果位置关系为 HEAD---R---W----TAIL，需要考虑 W 到 TAIL 之间的空间是否足够
        if(wpos >= rpos) {
            int32_t left = header_.size - wpos;
            int32_t size = left < totals ? left : totals;
            putfp_->save(msg->buf, size);
            if(size < totals) {
                putfp_->seek(sizeof(header_), SEEK_SET);
                putfp_->save((char* )msg->buf + size, totals - size);
            }
            if(padding > 0)
                putfp_->seek(padding, SEEK_CUR);
        }
        // 如果位置关系为 HEAD---W---R---TAIL，只需要考虑 W 到 R 之间的空间
        else {
            putfp_->save(msg->buf, totals);
            if(padding > 0)
                putfp_->seek(padding, SEEK_CUR);
        }
        header_.wpos += needed;
        if(header_.wpos % header_.size == 0)
            putfp_->seek(sizeof(header_), SEEK_SET);

        header_.wcnt++;

        update();
        return true;
    }

    xbus_msg* binlog::load()
    {
        if(header_.wpos <= header_.rpos + sizeof(int32_t) + sizeof(int32_t))
            return NULL;

        off_t wpos = header_.wpos % header_.size;
        off_t rpos = header_.rpos % header_.size;

        if(header_.wpos == header_.rpos)
            return NULL;
        else if(wpos > rpos) {
            // 如果位置关系为 HEAD---R---W---TAIL，那么需要考虑 R 和 W 之间的空间
            int32_t len = 0;
            int32_t tag = 0;
            getfp_->load(&len, sizeof(len));
            // 计算 sizeof(len) 对齐的时候需要将长度字节计算在内。当时因为已经读取了
            // 长度信息，所以 required 需要减去 sizeof(len)
            int32_t required = ceil(len + sizeof(tag) + sizeof(len), sizeof(len)) - sizeof(len);
            assert(header_.rpos + sizeof(len) + required <= header_.wpos);

            char* buf = cache_;
            if(required > CACHE_SIZE) {
                buf = new char[required];
            }

            getfp_->load(buf, required);
            header_.rpos += (sizeof(len) + required);
            if(header_.rpos % header_.size == 0)
                getfp_->seek(sizeof(header_), SEEK_SET);

            memcpy(&tag, buf, sizeof(tag));
            xbus_msg* msg = xbus_msg_init(tag, (char* )buf + sizeof(tag), len);

            if(buf != cache_)
                delete[] buf;

            update();
            header_.rcnt++;
            return msg;
        }
        // 如果位置关系为 HEAD---W---R---TAIL，只需要考虑 R 和 TAIL 之间的空间
        else {
            int32_t len = 0;
            int32_t tag = 0;

            int32_t left = header_.size - rpos;
            assert(left >= sizeof(len));
            getfp_->load(&len, sizeof(len));
            left -= sizeof(len);
            // 计算 sizeof(len) 对齐的时候需要将长度字节计算在内。当时因为已经读取了
            // 长度信息，所以 required 需要减去 sizeof(len)
            int32_t required = ceil(len + sizeof(tag) + sizeof(len), sizeof(len)) - sizeof(len);
            assert(header_.rpos + sizeof(len) + required <= header_.wpos);

            char* buf = cache_;
            if(required > CACHE_SIZE)
                buf = new char[required];

            int32_t size = left < required ? left : required;
            getfp_->load(buf, size);
            if(size < required) {
                getfp_->seek(sizeof(header_), SEEK_SET);
                getfp_->load((char* )buf + size, required - size);
            }

            memcpy(&tag, buf, sizeof(tag));
            header_.rpos += (sizeof(len) + required);
            if(header_.rpos % header_.size == 0)
                getfp_->seek(sizeof(header_), SEEK_SET);

            xbus_msg* msg = xbus_msg_init(tag, (char* )buf + sizeof(tag), len);

            if(buf != cache_)
                delete[] buf;

            update();
            header_.rcnt++;
            return msg;
        }
    }

    bool binlog::full() const
    {
        return header_.wpos + sizeof(int32_t) >= header_.rpos + header_.size;
    }

    off_t binlog::used() const
    {
        return header_.wpos - header_.rpos;
    }

    off_t binlog::size() const
    {
        return header_.size;
    }

    bool binlog::empty() const
    {
        return header_.rpos == header_.wpos;
    }

    const binlog::header* binlog::stat() const
    {
        return &header_;
    }

    void binlog::remove()
    {
        delete hdrfp_;
        delete getfp_;
        delete putfp_;
        hdrfp_ = NULL;
        getfp_ = NULL;
        putfp_ = NULL;
        binlog::remove(this->fname_.c_str());
    }

    void binlog::update()
    {
        if(count_++ % FLUSH_ITER == 0) {
            hdrfp_->seek(0, SEEK_SET);
            hdrfp_->save(&header_, sizeof(header_));
            hdrfp_->flush();
            putfp_->flush();
        }
    }

    void binlog::remove(const char *name)
    {
        ::unlink(name);
    }

    binlog::checkpoint binlog::mounted()
    {
        header* cp = new header(header_);
        return cp;
    }

    void binlog::discard(checkpoint cp)
    {
        header* hd = (header* )cp;
        delete hd;
    }

    bool binlog::restart(checkpoint cp, CP_TYPE type)
    {
        header* hd = (header* )cp;
        if(header_.wpos >= hd->rpos + hd->size) {
            delete hd;
            return false;
        }
        if(header_.rpos >= hd->rpos + hd->size) {
            delete hd;
            return false;
        }

        memcpy(&header_, hd, sizeof(header_));
        delete hd;

        hdrfp_->flush();

        if((type & READER) == READER) {
            getfp_->seek(header_.rpos % header_.size + sizeof(header_), SEEK_SET);
        }
        if((type & WRITER) == WRITER) {
            putfp_->seek(header_.wpos % header_.size + sizeof(header_), SEEK_SET);
        }

        return true;
    }

    ////////////////////////////////////////////////////////////////////////////
    static bool is_file_exist(const char *name)
    {
        struct stat buf;
        if(::stat(name, &buf) == 0)
            return true;
        return false;
    }


}

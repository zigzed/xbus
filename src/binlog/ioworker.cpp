/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "ioworker.h"
#include "common/sys/error.h"
#include "common/ipc/mmap.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

namespace bus {

    class stdio_file : public file_base {
    public:
        stdio_file();
        bool    open(const char *file, size_t cache);
        off_t   seek(off_t pos, int whence);
        bool    size(off_t size);
        size_t  load(void *buf, size_t len);
        size_t  save(const void *buf, size_t len);
        bool    flush();
        void    close();
    private:
        FILE*       file_;
        std::string name_;
    };

    class shmap_file : public file_base {
    public:
        shmap_file();
        bool    open(const char* file, size_t cache);
        off_t   seek(off_t pos, int whence);
        bool    size(off_t size);
        size_t  load(void* buf, size_t len);
        size_t  save(const void* buf, size_t len);
        bool    flush();
        void    close();
    private:
        enum { CACHE_SIZE = 2 * 1024 * 1024 };
        cxx::ipc::file_mapping*     shmfile_;
        cxx::ipc::mapped_region*    mapping_;
        off_t                       cur_pos_;
        bool                        isdirty_;
        off_t                       map_off_;
        off_t                       map_len_;
        off_t                       shm_len_;
        size_t                      caching_;
    };


    file_base* file_base::create()
    {
        return new shmap_file();
    }

    ////////////////////////////////////////////////////////////////////////////
    shmap_file::shmap_file() : shmfile_(NULL), mapping_(NULL),
        cur_pos_(0), isdirty_(false), map_off_(0), map_len_(0), shm_len_(0),
        caching_(0)
    {
    }

    bool shmap_file::open(const char *file, size_t cache)
    {
        caching_ = CACHE_SIZE;
        if(cache > 0)
            caching_ = cache;

        shmfile_ = new cxx::ipc::file_mapping(file, cxx::ipc::memory_mappable::ReadWrite);
        shm_len_ = shmfile_->size();
        mapping_ = new cxx::ipc::mapped_region(*shmfile_, cxx::ipc::mapped_region::ReadWrite, 0, caching_);
        map_off_ = mapping_->offset();
        map_len_ = mapping_->size();
    }

    off_t shmap_file::seek(off_t pos, int whence)
    {
        if(whence == SEEK_END)
            pos += shm_len_;
        else if(whence == SEEK_CUR)
            pos += cur_pos_;

        cur_pos_ = pos;

        return cur_pos_;
    }

    bool shmap_file::size(off_t size)
    {
        shmfile_->size(size);
        shm_len_ = shmfile_->size();
        assert(shm_len_ == size);
        return true;
    }

    size_t shmap_file::load(void *buf, size_t len)
    {
        if(shm_len_ == 0) {
            shm_len_ = shmfile_->size();
            if(shm_len_ < map_len_)
                map_len_ = shm_len_;
        }
        if(cur_pos_ + len > shm_len_)
            len = shm_len_ - cur_pos_;

        size_t done = 0;
        while(len > done && cur_pos_ < shm_len_) {
            if(cur_pos_ >= map_off_ && cur_pos_ < map_off_ + map_len_) {
                size_t left = map_len_ - (cur_pos_ - map_off_);
                size_t used  = (left > len - done) ? (len - done) : left;
                isdirty_ = true;
                void* mp = mapping_->data();
                off_t of = cur_pos_ - map_off_;
                memcpy((char* )buf + done, (char* )mp + of, used);
                cur_pos_    += used;
                done        += used;
            }
            else {
                flush();
                mapping_->move(cur_pos_, caching_);
                map_off_ = mapping_->offset();
                map_len_ = mapping_->size();
            }
        }
        return done;
    }

    size_t shmap_file::save(const void *buf, size_t len)
    {
        if(shm_len_ == 0) {
            shm_len_ = shmfile_->size();
            if(shm_len_ < map_len_)
                map_len_ = shm_len_;
        }
        if(cur_pos_ + len > shm_len_)
            len = shm_len_ - cur_pos_;

        size_t done = 0;
        while(len > done && cur_pos_ < shm_len_) {
            if(cur_pos_ >= map_off_ && cur_pos_ < map_off_ + map_len_) {
                size_t left = map_len_ - (cur_pos_ - map_off_);
                size_t used  = (left > len - done) ? (len - done) : left;
                isdirty_ = true;
                void* mp = mapping_->data();
                off_t of = cur_pos_ - map_off_;
                memcpy((char* )mp + of, (char* )buf + done, used);
                cur_pos_    += used;
                done        += used;
            }
            else {
                flush();
                mapping_->move(cur_pos_, caching_);
                map_off_ = mapping_->offset();
                map_len_ = mapping_->size();
            }
        }
        return done;
    }

    bool shmap_file::flush()
    {
        if(isdirty_) {
            isdirty_ = false;
            mapping_->commit();
        }
        return true;
    }

    void shmap_file::close()
    {
        flush();
        delete mapping_;
        mapping_ = NULL;
        delete shmfile_;
        shmfile_ = NULL;
    }

    ////////////////////////////////////////////////////////////////////////////
    stdio_file::stdio_file() : file_(NULL)
    {
    }

    bool stdio_file::open(const char *file, size_t)
    {
        name_ = file;
        close();
        // 文件打开方式不能使用 a+，因为 a+ 模式下所有的写操作都是 append，seek 对后续
        // 的写操作没有影响
        file_ = ::fopen(file, "a+");
        file_ = freopen(file, "r+", file_);
        return file_ != NULL;
    }

    off_t stdio_file::seek(off_t pos, int whence)
    {
        int rc = ::fseeko(file_, pos, whence);
        if(rc == 0)
            return ftello(file_);
        return rc;
    }

    bool stdio_file::size(off_t size)
    {
#ifndef OS_WINDOWS
        int rc = ::truncate(name_.c_str(), size);
        return rc == 0;
#else
        off_t len = ftell(file_);
        if(len < size) {
            char    temp[512];
            off_t   byte = size - len > 512 ? 512 : size - len;
            save(temp, byte);
            len += byte;
        }
        len = ftell(file_);
        return len == size;
#endif
    }

    size_t stdio_file::load(void *buf, size_t len)
    {
        size_t rc = ::fread(buf, 1, len, file_);
        return rc;
    }

    size_t stdio_file::save(const void *buf, size_t len)
    {
        size_t rc = ::fwrite(buf, 1, len, file_);
        return rc;
    }

    bool stdio_file::flush()
    {
        int rc = fflush(file_);
        return rc == 0;
    }

    void stdio_file::close()
    {
        if(file_) {
            ::fclose(file_);
            file_ = NULL;
        }
    }
            

}

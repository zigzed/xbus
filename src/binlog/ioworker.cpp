/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "ioworker.h"
#include "common/sys/error.h"
#include <stdio.h>
#include <unistd.h>

namespace bus {

    class stdio_file : public file_base {
    public:
        stdio_file();
        bool    open(const char *file);
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


    file_base* file_base::create()
    {
        return new stdio_file();
    }

    ////////////////////////////////////////////////////////////////////////////
    stdio_file::stdio_file() : file_(NULL)
    {
    }

    bool stdio_file::open(const char *file)
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

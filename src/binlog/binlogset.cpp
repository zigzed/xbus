/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "binlog.h"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <algorithm>
#include "common/sys/filesystem.h"
#include "common/str/tokenizer.h"

namespace bus {

    static int do_mkdir(const char* path, mode_t mode);
    static int do_mkdir_tree(const char* path, mode_t mode);

    binlogset::binlogset(const char *dir, off_t size, size_t count)
        : file_(dir), size_(size), count_(count), maxid_(1),
          writer_(NULL), reader_(NULL)
    {
        do_mkdir_tree(dir, 0755);
        scan_dir();
    }

    binlogset::~binlogset()
    {
        for(size_t i = 0; i < logs_.size(); ++i) {
            delete logs_[i];
        }
        logs_.clear();
    }

    bool binlogset::save(xbus_msg *msg)
    {
        if(writer_ == NULL) {
            for(size_t i = 0; i < logs_.size(); ++i) {
                if(!logs_[i]->full()) {
                    writer_ = logs_[i];
                    break;
                }
            }
        }
        if(writer_ == NULL) {
            writer_ = make_log();
        }
        if(!writer_)
            return false;
        if(writer_->full())
            return false;

        return writer_->save(msg);
    }

    xbus_msg* binlogset::load()
    {
        if(reader_ == NULL) {
            for(size_t i = 0; i < logs_.size(); ++i) {
                if(!logs_[i]->empty()) {
                    reader_ = logs_[i];
                    break;
                }
            }
        }
        if(reader_ == NULL) {
            reader_ = make_log();
        }
        if(!reader_)
            return NULL;
        if(reader_->empty())
            return NULL;

        return reader_->load();
    }

    bool binlogset::full() const
    {
        if(logs_.size() < size_)
            return false;
        for(size_t i = 0; i < logs_.size(); ++i) {
            if(!logs_[i]->full())
                return false;
        }
        return true;
    }

    bool binlogset::empty() const
    {
        if(logs_.empty())
            return true;

        for(size_t i = 0; i < logs_.size(); ++i) {
            if(!logs_[i]->empty())
                return false;
        }
        return true;
    }

    off_t binlogset::size() const
    {
        return size_ * count_;
    }

    off_t binlogset::used() const
    {
        off_t used_bytes = 0;
        for(size_t i = 0; i < logs_.size(); ++i) {
            used_bytes += logs_[i]->used();
        }
        return used_bytes;
    }

    void binlogset::scan_dir()
    {
        std::vector<std::string >   files;
        cxx::sys::DirIterator dir(file_.c_str());
        while(dir != cxx::sys::DirIterator()) {
            std::string file = dir.name();
            cxx::str::tokenizer token(file, ".");
            if(token.size() != 2) {
                ++dir;
                continue;
            }
            if(token[1] != "binlog") {
                ++dir;
                continue;
            }
            size_t id = atol(token[0].c_str());
            if(id == 0) {
                ++dir;
                continue;
            }
            if(id >= maxid_)
                maxid_ = id + 1;
            files.push_back(dir.path().name());

            ++dir;
        }

        // 保证binlog加载的顺序
        std::sort(files.begin(), files.end());
        for(size_t i = 0; i < files.size(); ++i) {
            size_t cache = 2 * 1024 * 1024;
            if(size_ < cache)
                cache = size_;
            binlog* blog = new binlog(files[i].c_str(), size_, cache);
            logs_.push_back(blog);
        }
        if(logs_.empty()) {
            make_log();
        }

        for(size_t i = 0; i < logs_.size(); ++i) {
            if(writer_ == NULL && !logs_[i]->full()) {
                writer_ = logs_[i];
            }
            if(reader_ == NULL && !logs_[i]->empty()) {
                reader_ = logs_[i];
            }
            if(writer_ != NULL && reader_ != NULL)
                break;
        }
    }

    binlog* binlogset::make_log()
    {
        clean_up();

        if(logs_.size() >= count_)
            return NULL;

        char fullname[255];
        sprintf(fullname, "%s/%08d.binlog", file_.c_str(), maxid_);
        maxid_++;

        size_t cache = 2 * 1024 * 1024;
        if(size_ < cache)
            cache = size_;
        binlog* blog = new binlog(fullname, size_, cache);
        logs_.push_back(blog);

        return blog;
    }

    void binlogset::clean_up()
    {
        std::vector<binlog* >   temp;
        while(!logs_.empty()) {
            binlog* head = logs_.front();
            // 如果 binlog 中没有数据，并且不是新创建的，那么就可以释放
            if(writer_ != head && reader_ != head && head->empty() &&
                    head->stat()->rcnt != 0 && head->stat()->wcnt != 0) {
                temp.push_back(head);
                logs_.pop_front();
            }
            else
                break;
        }
        for(size_t i = 0; i < temp.size(); ++i) {
            temp[i]->remove();
            delete temp[i];
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    static int do_mkdir(const char *path, mode_t mode)
    {
        struct stat st;
        if(stat(path, &st) != 0) {
            if(mkdir(path, mode) != 0 && errno != EEXIST)
                return -1;
        }
        else if(!S_ISDIR(st.st_mode)) {
            errno = ENOTDIR;
            return -1;
        }
        return 0;
    }

    static int do_mkdir_tree(const char *path, mode_t mode)
    {
        int status = 0;
        char* temp = strdup(path);
        char* sp = NULL;
        char* pp = temp;
        while(status == 0 && (sp = strchr(pp, '/')) != 0) {
            if(sp != pp) {
                *sp = '\0';
                status = do_mkdir(temp, mode);
                *sp = '/';
            }
            pp = sp + 1;
        }
        if(status == 0)
            status = do_mkdir(path, mode);
        free(temp);

        return status;
    }

}

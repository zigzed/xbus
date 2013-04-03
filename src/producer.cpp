/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "producer.h"
#include "common/con/net.h"
#include "common/con/fd.h"
#include "common/con/channel.h"
#include "xbus_msg.h"

namespace bus {

//    class simple_producer : public producer {
//    public:
//        typedef cxx::con::channel<xbus_msg* >   chan_t;
//        simple_producer(cxx::con::scheduler* s, const char* topic, const char* addr, unsigned short port, xbus_cfg* cfg);
//        void stop();
//    private:
//        static void sending(cxx::con::coroutine* c, void* p);
//        static void reading(cxx::con::coroutine* c, void* p);

//        cxx::con::scheduler*    contask_;
//        bool                    running_;
//        cxx::net::fd_t          sockets_;
//        chan_t                  channel_;
//    };

//    class window_producer : public producer {

//    };


//    //------------------------------------------------------------------------//
//    struct sending_arg {
//        std::string                 addr;
//        unsigned short              port;
//        simple_producer::chan_t*    chan;
//        bool*                       run;
//        std::string                 topic;
//    };

//    struct reading_arg {
//        std::string                 topic;
//        int64_t                     off;
//        simple_producer::chan_t*    chan;
//        bool*                       run;
//    };

//    simple_producer::simple_producer(cxx::con::scheduler *s, const char *topic, const char *addr, unsigned short port, xbus_cfg *cfg)
//        : producer(addr, port, cfg), contask_(s), sockets_(-1),
//          channel_(256)
//    {
//        sending_arg arg;
//        arg.addr = addr;
//        arg.port = port;
//        arg.chan = &channel_;
//        arg.run  = &running_;
//        arg.topic= topic;
//        s->spawn(simple_producer::sending, &arg);
//    }


//    void simple_producer::stop()
//    {
//        running_ = false;
//    }

//    void simple_producer::reading(cxx::con::coroutine *c, void *p)
//    {
//        reading_arg* arg = (reading_arg* )p;
//        std::string topic = arg->topic;
//        simple_producer::chan_t* chan = arg->chan;
//        int64_t off = arg->off;
//        bool*   run = arg->run;

////        cxx::con::filer f(c, topic.c_str());
////        f.seek(off, SEEK_SET);

////        while(*run) {
////            int32_t len = 0;
////            int32_t tag = 0;
////            if(f.load((char* )&len, sizeof(len)) <= 0)
////                break;
////            if(f.load((char* )&tag, sizeof(tag)) <= 0)
////                break;

////            xbus_msg* msg = xbus_msg_init(tag, NULL, len - sizeof(tag) - sizeof(len));
////            if(f.load((char* )msg->buf, msg->len) <= 0)
////                break;
////            msg->off = off;
////            chan->send(c, msg, 0);
////            off += len;
////        }

////        f.close();
//    }

//    void simple_producer::sending(cxx::con::coroutine *c, void *p)
//    {
//        sending_arg* arg = (sending_arg* )p;
//        std::string     addr = arg->addr;
//        unsigned short  port = arg->port;
//        simple_producer::chan_t* chan = arg->chan;
//        bool*           run  = arg->run;
//        std::string     topic= arg->topic;

////        while(*run) {

////            cxx::con::connector connector(c, true);
////            cxx::net::fd_t fd = connector.connect(addr.c_str(), port, 5000);
////            if(fd == -1)
////                continue;

////            // 连接成功后发送 producer 的类型，并等待 consumer 发送偏移量
////            cxx::con::socketor  sender(c, fd);
////            int32_t ver = 0;
////            int64_t off = 0;
////            if(sender.send((char* )&ver, sizeof(ver)) <= 0) {
////                sender.close();
////                break;
////            }
////            // 如果超时没有收到 consumer 发送的偏移量，则断开连接
////            if(sender.recv((char* )&off, sizeof(off), 1000) <= 0) {
////                sender.close();
////                break;
////            }

////            // 协商完毕后，按照指定的偏移量读取数据
////            reading_arg arg;
////            arg.chan = chan;
////            arg.off  = off;
////            arg.run  = run;
////            arg.topic= topic;
////            c->sched()->spawn(simple_producer::reading, &arg);

////            // 根据读取的数据发送
////            xbus_msg* msg = NULL;
////            while(chan->recv(c, msg)) {
////                int32_t total_len = msg->len + sizeof(msg->tag) + sizeof(msg->off);
////                if(sender.send((char* )&total_len, sizeof(total_len)) <= 0)
////                    break;
////                if(sender.send((char* )&msg->tag, sizeof(msg->tag)) <= 0)
////                    break;
////                if(sender.send((char* )&msg->off, sizeof(msg->off)) <= 0)
////                    break;
////                if(sender.send((char* )msg->buf, msg->len) <= 0)
////                    break;

////                xbus_msg_free(msg);
////                msg = NULL;
////            }

////            if(msg)
////                xbus_msg_free(msg);

////            sender.close();
////        }
//    }




}

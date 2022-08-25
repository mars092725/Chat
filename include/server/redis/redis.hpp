#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
using namespace std;

class Redis
{
public:
    Redis();
    ~Redis();

    // 连接redis服务器 
    bool connect();

    // 发布消息
    bool publish(int channel, string message);

    // 订阅消息
    bool subscribe(int channel);

    // 取消订阅消息
    bool unsubscribe(int channel);

    // 接收订阅通道中的消息
    void observer_channel_message();

    // 回调函数
    void init_notify_handler(function<void(int, string)> fn);

private:
    // hiredis同步上下文对象，负责publish消息
    // 相当于redis-cli的连接信息
    redisContext *_publish_context;

    // hiredis同步上下文对象，负责subscribe消息
    redisContext *_subcribe_context;

    // 回调操作，收到订阅的消息，给service层上报
    // 其实就是当有消息到达，就会触发事件，这个事件就是这个回调函数
    // 该回调函数中的int指的是通道号，string指的是message
    function<void(int, string)> _notify_message_handler;
};

#endif

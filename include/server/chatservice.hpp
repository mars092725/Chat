#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <iostream>
#include <unordered_map>
#include <json.hpp>
#include <functional>
#include <mutex>
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;
// 注册的业务方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

// 一个消息id映射一段业务代码
//
class ChatService {
// 主要做业务，采用单例模式实现
public:
    // 获取单例对象的接口函数
    static ChatService* instance();

    // 处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 处理注销业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    
    // 处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 创建群组
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 加入群组
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 群组聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time); 

    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);

    // 服务器异常，业务重置方法
    void reset();

    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);

    // redis的回调函数，获取redis消息队列中的订阅的消息
    void handleRedisSubscribeMessage(int, string);

private:
    // 单例模式，构造函数私有化
    ChatService();
    // 存储消息ID对应的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;

    // 存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;

    // 定义互斥锁，保证_userConnMap的线程安全
    mutex _connMutex;

    // 数据操作对象
    UserModel _userModel;
    OfflineMessage _offlineMessageModel;
    FriendModel _fiendModel;
    GroupModel _groupModel;

    // redis操作对象
    Redis _redis;
};

#endif

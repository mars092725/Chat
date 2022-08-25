#include "chatserver.hpp"
#include "chatservice.hpp"
#include "json.hpp"

using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* loop,
		       const InetAddress &listenAddr,
		       const string &nameArg)
		:_server(loop, listenAddr, nameArg), _loop(loop) {
		
    // 注册连接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
        
    // 注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1,_2,_3));

    // 设置线程数量
    _server.setThreadNum(4);
		
}

void ChatServer::start() {
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn) {
    if(!conn->connected()){
	// 客户端异常关闭
	ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn,
		           Buffer *buffer,
			   Timestamp time) {

    string buf = buffer->retrieveAllAsString();
    // 进行数据反序列化
    json js = json::parse(buf);
    
    // 通过js["msgid"]获取一个业务处理器handler，实现网络模块和业务模块进行解偶
    // 业务handler 需要conn,js,time
    // get将js中的数据转化为要求类型
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    // 回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn, js, time);
}


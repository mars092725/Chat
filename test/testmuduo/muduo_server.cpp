#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;
/*
// moduo给用户提供了两个主要的类：TcpServer/TcpClient
// TcpClient：用于编写服务器程序
// Tcperver：用于编写客户端程序
// 直接使用muduo好处：将网络IO的代码和业务代码区分来
// 业务代码只关注用户的连接和断开，用户的读写事件
*/

#include <iostream> 
#include <string>
#include <functional>  // 绑定器
using namespace std;
using namespace placeholders;  // 占位符
/*

 *基于muduo网络库的服务器程序
1. 组合TcpServer对象
2. 创建EventLoop事件循环对象的指针
3. 明确TcpServer构造函数需要什么参数，写出构造函数
4. 在当前服务器类的构造函数当中注册处理连接的回调函数和处理读写事件的回调函数
5. 设置合适的服务器端线程数量，muduo库会自己划分IO线程和work线程
*/

class ChatServer {

public:
	ChatServer(EventLoop* loop,
		   const InetAddress &listenAddr,  // IP+端口号
		   const string &nameArg)  // 服务器名称
	    :_server(loop, listenAddr, nameArg), _loop(loop)
	{
	    // 给服务器注册用户连接的创建和断开回调
	    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

	    // 给服务器注册用户读写事件回调
	    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
	
	    // 设置服务器端的线程数量
	    // 如果没有设置，默认只有一个线程，该线程既要负责连接的创建还要处理事件读写
	    // 如果设置多个，默认其中一个线程用于处理连接
	    _server.setThreadNum(4);  
	}

	void start() {
	    _server.start();
	}

private:
	// 专门处理用户连接和创建
	void onConnection(const TcpConnectionPtr &conn) {
	    if(conn->connected()) {
	        cout << conn->peerAddress().toIpPort() << "->"  << conn->localAddress().toIpPort() << "state:online" << endl;
	    }else{
	        cout << conn->peerAddress().toIpPort() << "->"  << conn->localAddress().toIpPort() << "state:offline" << endl; 
	        conn->shutdown();  // 连接断开，回收fd，相当于close(fd)
		//_loop->quit();  相当于服务器结束
	    }
	}

	// 专门用来处理用户的读写事件
	void onMessage(const TcpConnectionPtr &conn,  // 连接
		       Buffer *buffer,  // 缓冲区
		       Timestamp time)  // 接收到数据的时间信息
	{
	    string buf = buffer->retrieveAllAsString();
	    cout << "recv data:" << buf << "time:" << time.toString() << endl;
	    conn->send(buf); 
	}

	TcpServer _server;
	EventLoop *_loop;    // epoll

};


int main() {
    EventLoop loop;
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&loop, addr, "ChatServer");
    
    server.start();
    loop.loop();  // epoll_wait以阻塞方式等待新用户连接，已连接用户的读写事件等。

    return 0;
}

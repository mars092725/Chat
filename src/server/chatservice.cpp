#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>
using namespace muduo;
using namespace std;
// 获取单例对象的接口函数
ChatService* ChatService::instance() {
    static ChatService service;
    return &service;
}

// 注册消息以及相应的回调操作
ChatService::ChatService() {
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    // 连接redis服务器
    if(_redis.connect()) {
        // 设置上报消息
	_redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));  // 第一个参数是通道号，第二各参数是message
    }


}

// 服务器异常，业务重置方法
void ChatService::reset(){
    // 将online的用户设置为offline
    _userModel.resetState();
}


// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid) {
    // 记录错误日志，msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end()) {
	// 为了防止处理抛出异常,返回一个默认的处理器
	return [=](const TcpConnectionPtr &conn, json &js, Timestamp) {
	    LOG_ERROR << "msgid:" << msgid << "con not find handler!";
	};
    }else{
        return _msgHandlerMap[msgid];
    }
}

// 处理登录业务 
// 这里就关系到了数据模块，我们要求业务模块中不能进行数据库的增删改查的操作，两者要进行解偶
// ORM object relation map 对象关系映射框架，使得业务层操作的都是对象，数据模块都是对于数据库的操作。
void ChatService::login(const TcpConnectionPtr& conn, json &js, Timestamp time) {
    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = _userModel.query(id);
    if(user.getId() == id && user.getPwd() == pwd) {
	// 该用户已经登录，不允许重复登录
	if(user.getState() != "offline") {
	    json response;
	    response["msgid"] = LOGIN_MSG_ACK;
	    response["errno"] = 2;
	    response["errmsg"] = "该账号已经登录，请重新登录新账号";
	} else {
        // 登录成功
	
	    // 登陆成功，记录用户链接信息
	    // 这里需要注意的是，由于会通过聊天服务器向用户发送数据，所以使用长连接
	    // 由于该map表将会被多个线程同时访问，会随着用户上线和下线而发生改变，所以这部分需要思考线程安全问题
	    {
		// 临界区尽量小，且guard锁可以自动施放
		lock_guard<mutex> lock(_connMutex);
		_userConnMap.insert({id, conn});
	    }

	    // id用户登录成功后，向redis订阅channel
	    _redis.subscribe(id);

	    // 更新相关信息state
	    user.setState("online");
            _userModel.updateState(user);

	    json response;
       	    response["msgid"] = LOGIN_MSG_ACK;
	    response["errno"] = 0;
	    response["id"] = user.getId();
	    response["name"] = user.getName();

	    // 查询该用户是否有离线消息
	    vector<string> vec = _offlineMessageModel.query(id);
	    if(!vec.empty()) {
	        response["offlinemsg"] = vec;
		_offlineMessageModel.remove(id);
	    }

	    // 查询该用户的好友信息并返回
	    vector<User> userVec = _fiendModel.query(id);
	    if(!userVec.empty()) {
		vector<string> vec2;
		for(User &user:userVec) {
		    json js;
		    js["id"] = user.getId();
		    js["name"] = user.getName();
		    js["state"] = user.getState();
		    vec2.push_back(js.dump());
		}

		response["friends"] = vec2;
	    }

	    conn->send(response.dump());	
	}
     } else {
    	// 该用户不存在，登录失败
	json response;
	response["msgid"] = LOGIN_MSG_ACK;
	response["errno"] = 1;
	conn->send(response.dump());
    }
}


// 处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    // 用户注销，相当于下线，再redis中取消订阅通道
    _redis.unsubscribe(userid);

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    if(!_userModel.updateState(user)) {
        cout << "login fail!" << endl;
    }
}

// 处理注册业务
void ChatService::reg(const TcpConnectionPtr& conn, json &js, Timestamp time) {

    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state =  _userModel.insert(user);
    if(state) {
        // 注册成功
	json response;
	response["msgid"] = REG_MSG_ACK;
	response["errno"] = 0;
	response["id"] = user.getId();
	conn->send(response.dump());	
    }else{
        // 注册失败
	json response;
	response["msgid"] = REG_MSG_ACK;
	response["errno"] = 1;
	conn->send(response.dump());
    }
}

// 客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn) {
    // 这也有对于map表中的操作，需要使用互斥锁保证其线程安全
    lock_guard<mutex> lock(_connMutex);
    User user;
    for(auto it=_userConnMap.begin(); it!=_userConnMap.end(); ++it) {
        if(it->second == conn) {
	    // 从map表删除相应的连接信息
	   user.setId(it->first);
	   _userConnMap.erase(it);
	    break;
	}
    }

    // 

    // 将用户的状态信息进行更新 online->offline
    if(user.getId()!=-1) {
   	 user.setState("offline");
   	 _userModel.updateState(user);
    } 
}

// 一对一聊天
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
	auto it = _userConnMap.find(toid);
	if(it != _userConnMap.end()) {
	    // toid在线，直接转发消息，集群环境此时就是两个client在同一个服务器上
	    // 服务器主动推送消息给toid用户
	    it->second->send(js.dump());
	    return;
	}
    }

    User user = _userModel.query(toid);
    if(user.getState() == "online") {
	// toid在线，但是_userConnMap中没有该用户，说明此时该用户在线，但是此时和该用户不在同一台服务器上。
        _redis.publish(toid, js.dump());
	return;
    }

    // toid不在线,存储离线消息
    _offlineMessageModel.insert(toid, js.dump());
}

// 添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 添加好友信息
    _fiendModel.insert(userid, friendid);
}

// 创建群组
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 存储新创建的群组信息
    // 因为此时群信息还没有添加到数据库，群id还没确定
    Group group(-1, name, desc);
    if(_groupModel.createGroup(group)) {
        // 群组创建好了之后，此时该群的创建人creator也就确定了
	// 添加群成员
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

// 加入群组
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    
    lock_guard<mutex> lock(_connMutex);
    // 转发过程中线程连接不可以断开，也可以放入循环中，放入循环中则是允许在转发过程中某个线程连接可以断开
    for(int id : useridVec) {
	auto it = _userConnMap.find(id);
	// 如果能从_userConnMap中找到该用户id，说明该用户在线；如果找不到就说明该用户离线
	if(it!=_userConnMap.end()) {
	    // 转发群消息
	    it->second->send(js.dump());
	}else {
 	    User user = _userModel.query(id);
	    if(user.getState() == "online") {
	        _redis.publish(id, js.dump());
	    }else {
	    // 存储离线群消息
	    _offlineMessageModel.insert(id, js.dump());
	    }
    	}
    }
}


void ChatService::handleRedisSubscribeMessage(int userid, string msg) {

    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    
    // 有可能出现redis在给用户发送消息的过程中，用户下线了
    if(it != _userConnMap.end()) {
        it->second->send(msg);
	return;
    }

    _offlineMessageModel.insert(userid, msg);
}

#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
using namespace std;

string func1() {
    json js; // 消息类型

    // 封装的消息结构
    // json底层使用链式哈希实现的，最终插入的数据是无序的。
    js["msg_type"] = 2;              // 消息类型
    js["from"] = "zhang san";	     // 消息发送者
    js["to"] = "li si";              // 消息接受者
    js["msg"] = "where are you";     // 消息内容
	
    // dump将消息进行序列化，将消息发送至网络
    string sendBuf = js.dump();
    // cout << sendBuf.c_str() << endl;
    return sendBuf;
}

string func2() {
    json js;
    // json中的数据类型可以是数组
    js["id"] = {1,2,3,4,5};
    js["name"] = "zhang san";
    // msg对应的value依旧是一个json类型的数据
    js["msg"]["bob"] = "hello alice";
    js["msg"]["alice"] = "hello bob";
    // 除了上文这种添加键值的方式还可以使用如下方式
    js["msg"] = {{"bob", "hello alice"}, {"alice", "hello bob"}};
    
    string sendBuf = js.dump();
    return sendBuf;
}

string func3() {
    json js;
    // 直接序列化一个数组
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);
    js["list"] = vec;

    // 直接序列化一个map容器
    map<int, string> m;
    m.insert({1, "mars"});
    m.insert({2, "hua"});
    m.insert({3, "hao"});

    js["path"] = m;
    string sendBuf = js.dump();
    return sendBuf;

}

int main(){
    string recvBuf1 = func1();
    json jsbuf1 = json::parse(recvBuf1);
    cout << jsbuf1["msg_type"] << " ";
    cout << jsbuf1["from"] << " ";
    cout << jsbuf1["to"] << " ";
    cout << jsbuf1["msg"] << endl;
    
    // 进行反序列化，json字符串反序列化时可以直接转化为数据对象，看作一个容器
    string recvBuf2 = func2();
    json jsbuf2 = json::parse(recvBuf2);
    cout << jsbuf2["id"] << endl;
    auto id = jsbuf2["id"];
    cout << id[0] << id[4] << endl;
    cout << jsbuf2["msg"] << endl;
    auto msg = jsbuf2["msg"];
    cout << msg["alice"] << msg["bob"] << endl;

  
    string recvBuf3 = func3();
    json jsbuf3 = json::parse(recvBuf3);
    // jsbuf中是一个数组类型，可以直接放入vector容器当中
    vector<int> vec = jsbuf3["list"];
    map<int, string> m = jsbuf3["path"];
   
    // js中的数组类型可以直接放入到vector容器当中
    return 0;
}

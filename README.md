# Chat
基于muduo实现的集群聊天服务器和客户端源码，使用mysql数据库存储相关数据。采用nginx实现负载均衡，结合redis发布-订阅模式来实现在不同服务器上客户端进行通信。 


在运行之前需要建立好mysql的数据表信息，配置好nginx并将其开启，还需要打开redis-cli，具体如下文所示。

编译方式如下：
1. cd build
2. rm -fr *
3. cmake ..
4. make


Mysql数据库设计：需要设计五张表，用于存放注册用户的User表，存放好友的Friend表，存放建立的群组信息表AllGroup，存放群组成员信息的GroupUser，存放当用户不在线时的所有离线信息OfflineMessage，表中的具体信息如下。
1. User: id(PRI) name  password  state("offline"/"online")
2. Friend: userid(联合主键)  friendif(联合主键)
3. AllGroup: id(PRI)  groupname  groupdesc
4. GroupUser: groupid(联合主键)  userid（联合主键） grouprole（"creator"/"normal"）
5. OfflineMessage: userid  message

关于nginx负载均衡的配置，修改nginx的配置信息，位于/usr/local/nginx/conf/nginx.conf，增加内容如下：
```bash
stream {
    upstream MyServer {
        server 127.0.0.1 6000 weight=1 max_fails=3 fail_timeout=30s;
        server 127.0.0.1 6002 weight=1 max_fails=3 fail_timeout=30s;
    }
    server {
        proxy_connect_timeout 1s;
        listen 8000;
        proxy_pass MyServer;
        tcp_nodelay on;
    }
}
```
文件夹：
1. bin：最终生成的可执行文件；
2. build：编译过程中产生的中间文件；
3. CMakeLists：Cmake文件
4. include：所有的头文件；
5. src：所有的源文件；
6. test：单独的测试性文件，主要用于测试json和muduo；
7. thirdparty：第三方文件json.hpp，用于进行序列化和反序列化；


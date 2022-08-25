#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"
#include <string>
#include <vector>
using namespace std;

// 维护群组信息的操作接口方法
class GroupModel
{
public:
    // 创建群组
    bool createGroup(Group &group);

    // 加入群组
    void addGroup(int userid, int groupid, string role);

    // 查询用户所在群组信息
    // 返回结果为该用户的所有群组信息即群组内的群成员信息
    vector<Group> queryGroups(int userid);

    // 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其它成员群发消息
    // 通过chatservice中的_userConnMap找到对应id的连接，进行群发消息
    vector<int> queryGroupUsers(int userid, int groupid);
};

#endif

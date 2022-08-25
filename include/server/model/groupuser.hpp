#ifndef GROUPUSER_H
#define GROUPUSER_H

// 因为在群中用户多了一个信息，即是角色信息
// 所以该类主要继承了User类，在此基础上增加了用户角色信息
#include "user.hpp"

// 群组用户，多了一个role角色信息，从User类直接继承，复用User的其它信息
class GroupUser : public User
{
public:
    void setRole(string role) { this->role = role; }
    string getRole() { return this->role; }

private:
    string role;  // 群族中的成员角色信息
};

#endif

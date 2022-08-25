#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

// user表的操作类，主要针对表的，其本质就是表的增删改查
class UserModel{
public:
	// user表的增加操作
	bool insert(User &user);

	// 根据用户ID查询用户信息
	User query(int id);

	// 更新业务信息
	bool updateState(User &user);

	// 重置用户的状态信息
	void resetState();
};

#endif

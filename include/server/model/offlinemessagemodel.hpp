#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <string>
#include <vector>
using namespace std;

// 提供离线消息表操作方法
class OfflineMessage{
public:
	// 存储用户的离线消息
	void insert(int userid, string msg);

	// 删除用户的离线消息
	void remove(int userid);

	// 查询用户的离线消息
	vector<string> query(int userid);

};

#endif

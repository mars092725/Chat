#include "friendmodel.hpp"
#include "db.h"

void FriendModel::insert(int userid, int friendid) {
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend values(%d, %d)", userid, friendid);

    MySQL mysql;
    if(mysql.connect()) {
        mysql.update(sql);
    }
}

vector<User> FriendModel::query(int userid) {
    char sql[1024] = {0};
    // 注意这里的联合查询
    sprintf(sql, "select a.id,a.name,a.state from User a inner join Friend b on b.friendid=a.id where b.userid = %d", userid);

    vector<User> vec;
    MySQL mysql;
    if(mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
	if(res != nullptr) {
	    MYSQL_ROW row;
	    while((row = mysql_fetch_row(res))!=nullptr) {
		// 用查询结构初始化一个User放入vec中作为uerid用户的用户列表
		User user;
		user.setId(atoi(row[0]));
		user.setName(row[1]);
		user.setState(row[2]);

		vec.push_back(user);
	    }
	    mysql_free_result(res);
	}
    }
    return vec;
}

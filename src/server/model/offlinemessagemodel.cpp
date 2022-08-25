#include "offlinemessagemodel.hpp"
#include "db.h"

void OfflineMessage::insert(int userid, string msg) {
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage values(%d, '%s')", userid, msg.c_str());

    MySQL mysql;
    if(mysql.connect()) {
        mysql.update(sql);
    }    
}

void OfflineMessage::remove(int userid) {
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid=%d", userid);

    MySQL mysql;
    if(mysql.connect()) {
        mysql.update(sql);
    }    

}

vector<string> OfflineMessage::query(int userid) {
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid=%d", userid);

    vector<string> vec;
    MySQL mysql;
    if(mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
	if(res!=nullptr) {
	    MYSQL_ROW row = mysql_fetch_row(res);
	    while(row != nullptr) {
		// 把userid用户的所有离线消息放入vec当中返回
	        vec.push_back(row[0]);
                row = mysql_fetch_row(res);
	    }
	    // 释放res的资源
	    mysql_free_result(res);
	    return vec;
	}
    }
    return vec;
}

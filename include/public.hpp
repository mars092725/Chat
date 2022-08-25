#ifndef PUBLIC_H
#define PUBLIC_H

/*
 * server和clent的公共文件
 * 1. 消息类型
 *
 */

enum EnMsgType {
    LOGIN_MSG = 1, // 登录消息  msgid id password
    LOGIN_MSG_ACK,
    LOGINOUT_MSG,  // 注销账号
    REG_MSG, // 注册消息        msgid name password
    REG_MSG_ACK, // 注册消息响应  
    ONE_CHAT_MSG, // 聊天信息   msgid id from to msg
    ADD_FRIEND_MSG, // 添加好友

    CREATE_GROUP_MSG, // 创建群组
    ADD_GROUP_MSG, // 加入群组
    GROUP_CHAT_MSG, // 群聊天
};

#endif

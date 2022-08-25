#include "db.h"
#include <muduo/base/Logging.h>

// 将数据模块的代码与业务模块进行解偶，也就是ORM
// 使得业务层操作的都是对象，数据模块将所有和数据库相关的操作进行操作
// 数据库配置信息
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";

// 初始化数据库连接
MySQL::MySQL()
{
    // 这并不是连接的初始化，此时还没有给连接提供用户密码，无法进行连接
    // 该函数功能只是为连接分配相关资源
    _conn = mysql_init(nullptr);
}

// 释放数据库连接资源
MySQL::~MySQL()
{
    if (_conn != nullptr)
	// 释放该连接所占的资源
        mysql_close(_conn);
}

// 连接数据库
bool MySQL::connect()
{
    // 此时建立连接，其中的参数意义从左往右意义如下：
    // 连接 + server-IP + 用户名 + 密码 + 数据库名 + 端口号3306 。。。
    // 如果返回值为0则表示连接成功
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                                  password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        // C和C++代码默认的编码字符是ASCII，如果不设置，从MySQL上拉下来的中文会显示“ ？”
        mysql_query(_conn, "set names gbk");
        LOG_INFO << "connect mysql success!";
    }
    else
    {
        LOG_INFO << "connect mysql fail!";
    }

    return p;
}

// 更新操作
bool MySQL::update(string sql) // 针对INSERT和DELETE操作
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                 << sql << "更新失败!";
        return false;
    }

    return true;
}

// 查询操作
MYSQL_RES *MySQL::query(string sql) // 针对SELECT操作
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                 << sql << "查询失败!";
        return nullptr;
    }
    
    return mysql_use_result(_conn);
}

// 获取连接
MYSQL* MySQL::getConnection()
{
    return _conn;
}

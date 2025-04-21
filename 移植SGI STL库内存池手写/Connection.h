#pragma once
#include <mysql.h>
#include "public.h"
#include<string>
#include<ctime>
/*
MySQL数据库操作
*/
class Connection {

public:
	Connection(); // 构造函数
	~Connection(); // 析构函数
	bool connect(std::string ip, unsigned short port, 
		std::string username,
		std::string password, 
		std::string dbname);
	bool update(std::string sql); // 更新数据
	MYSQL_RES* query(std::string sql); // 查询数据
	void RefreshAliveTime() {
		_alivetime = clock(); // 更新连接存活时间
	}
	clock_t getAliveTime() const{
		return clock() - _alivetime; // 返回连接存活时间
	} // 获取连接存活时间
private:
	MYSQL* _conn; // 表示和MySQL Server的一条连接
	clock_t _alivetime;//记录进入空闲状态的起始存活时间
};
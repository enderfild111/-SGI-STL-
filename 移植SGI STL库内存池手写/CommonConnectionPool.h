#pragma once
/*
连接池功能模块
//单例模式
*/
#include <list>
#include <mutex>
#include <condition_variable>
#include<string>
#include<queue>
#include<atomic>
#include<thread>
#include<iostream>
#include"Connection.h"
using namespace std;
class CommonConnectionPool
{
public:
	//获取连接池实例
	static CommonConnectionPool* getConnectionPool();
private:
	//构造函数私有化
	CommonConnectionPool();
	bool loadConfigFile();//加载配置文件
private:
	string _ip;//数据库IP地址
	unsigned short _port;//数据库端口号
	string _username;//数据库用户名
	string _password;//数据库密码
	string _dbname;//数据库名称
	int _initSize;//连接池初始化大小
	int _maxSize;//连接池最大大小
	int _maxIdleTime;//连接最大空闲时间
	int _connectionTimeout;//连接超时时间
	atomic_int _connectionCnt;//连接池当前连接数
	queue<Connection*> _connectionQue;//连接池
	mutex _queueMutex;//连接池互斥锁
};
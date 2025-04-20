#pragma once
#include <mysql.h>
#include "public.h"
#include<string>
/*
MySQL���ݿ����
*/
class Connection {

public:
	Connection(); // ���캯��
	~Connection(); // ��������
	bool connect(std::string ip, unsigned short port, 
		std::string username,
		std::string password, 
		std::string dbname);
	bool update(std::string sql); // ��������
	MYSQL_RES* query(std::string sql); // ��ѯ����
private:
	MYSQL* _conn; // ��ʾ��MySQL Server��һ������
};
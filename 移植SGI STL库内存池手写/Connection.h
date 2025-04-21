#pragma once
#include <mysql.h>
#include "public.h"
#include<string>
#include<ctime>
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
	void RefreshAliveTime() {
		_alivetime = clock(); // �������Ӵ��ʱ��
	}
	clock_t getAliveTime() const{
		return clock() - _alivetime; // �������Ӵ��ʱ��
	} // ��ȡ���Ӵ��ʱ��
private:
	MYSQL* _conn; // ��ʾ��MySQL Server��һ������
	clock_t _alivetime;//��¼�������״̬����ʼ���ʱ��
};
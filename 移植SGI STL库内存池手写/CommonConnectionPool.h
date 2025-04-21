#pragma once
/*
���ӳع���ģ��
//����ģʽ
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
	//��ȡ���ӳ�ʵ��
	static CommonConnectionPool* getConnectionPool();
private:
	//���캯��˽�л�
	CommonConnectionPool();
	bool loadConfigFile();//���������ļ�
private:
	string _ip;//���ݿ�IP��ַ
	unsigned short _port;//���ݿ�˿ں�
	string _username;//���ݿ��û���
	string _password;//���ݿ�����
	string _dbname;//���ݿ�����
	int _initSize;//���ӳس�ʼ����С
	int _maxSize;//���ӳ�����С
	int _maxIdleTime;//����������ʱ��
	int _connectionTimeout;//���ӳ�ʱʱ��
	atomic_int _connectionCnt;//���ӳص�ǰ������
	queue<Connection*> _connectionQue;//���ӳ�
	mutex _queueMutex;//���ӳػ�����
};
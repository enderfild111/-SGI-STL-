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
#include<memory>
#include<functional>
#include"Connection.h"
using namespace std;
class CommonConnectionPool
{
public:
	//��ȡ���ӳ�ʵ��
	static CommonConnectionPool* getConnectionPool();
	//���ⲿ�ṩ�ӿڣ���ȡ���õ�����
	shared_ptr<Connection> getConnection();//ͨ������ָ�뷵�����ӣ�֮���ض���shared_ptr��ɾ��������������Դ�������ӳ�
private:
	//���캯��˽�л�
	CommonConnectionPool();
	~CommonConnectionPool();
	bool loadConfigFile();//���������ļ�
	//�����ڶ������߳���ר��������������
	void produceConnectionTask();
	void scannerConnerctionTask();
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
	condition_variable _cv;//���ӳ���������,����֪ͨ�������߳�
};
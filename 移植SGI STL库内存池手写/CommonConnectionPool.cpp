#include "CommonConnectionPool.h"
//���캯������������
CommonConnectionPool::CommonConnectionPool()
{
	if (!loadConfigFile()) {
		return;
	}
	/*
	������ʼ����������
	*/
	for (int i = 0; i < _initSize; i++) {
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		_connectionQue.push(p);
		_connectionCnt++;
	}
	//����һ���µ��̣߳���Ϊ���ӵ�������
}

CommonConnectionPool::~CommonConnectionPool()
{
}
//�̰߳�ȫ����������ģʽ
CommonConnectionPool* CommonConnectionPool::getConnectionPool() {
	static CommonConnectionPool pool;//��̬�ֲ��������������Զ�lock��unlock���̰߳�ȫ
	return &pool;
} 
bool CommonConnectionPool::loadConfigFile() {
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == nullptr) {
		LOG("mysql.ini not found");
		return false;
	}
	while (!feof(pf)) {
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		string str = line;
		int idx = str.find("=",0);
		if (idx == -1) {//���л���ע����
			continue;
		}
		//����key-value
		//password = root\n
		int endIdx = str.find("\n", idx);
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endIdx - idx - 1);
		if (key == "ip") {
			_ip = value;
		}
		else if (key == "port") {
			_port = atoi(value.c_str());
		}
		else if (key == "dbname") {
			_dbname = value;
		}
		else if (key == "username") {
			_username = value;
		}
		else if (key == "password") {
			_password = value;
		}
		else if (key == "initSize") {
			_initSize = atoi(value.c_str());
		}
		else if (key == "maxSize") {
			_maxSize = atoi(value.c_str());
		}
		else if (key == "maxIdleTime") {
			_maxIdleTime = atoi(value.c_str());
		}
		else if (key == "connectionTimeout") {
			_connectionTimeout = atoi(value.c_str());
		}
	}
	return true;
}
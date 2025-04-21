#include "CommonConnectionPool.h"
//���캯������������
CommonConnectionPool::CommonConnectionPool()
{
	if (!loadConfigFile()) {
		LOG("�����ļ�����ʧ��");
		return;
	}
	/*
	������ʼ����������
	*/
	for (int i = 0; i < _initSize; i++) {
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->RefreshAliveTime();//ÿ�ν�����У�ˢ�¿��д��ʱ��
		_connectionQue.push(p);
		_connectionCnt++;
		//cout << "������" << i + 1 << "������" << endl;
	}
	//����һ���µ��̣߳���Ϊ���ӵ�������
	//���ð�������һ������ĳ�Ա������һ���߳���
	thread produce(std::bind(&CommonConnectionPool::produceConnectionTask, this));
	produce.detach();//�̷߳��룬���������߳�
	//����һ���µĶ�ʱ�̣߳�ɨ�賬��_maxIdleTime�Ŀ������ӣ����ж������ӵĻ���
	thread scanner(std::bind(&CommonConnectionPool::scannerConnerctionTask, this));
	scanner.detach();//�̷߳��룬���������߳�
}

CommonConnectionPool::~CommonConnectionPool()
{

}
//�̰߳�ȫ����������ģʽ
CommonConnectionPool* CommonConnectionPool::getConnectionPool() {
	static CommonConnectionPool pool;//��̬�ֲ��������������Զ�lock��unlock���̰߳�ȫ
	return &pool;
} 
void CommonConnectionPool::scannerConnerctionTask() {
	for (;;) {
		//sleepģ�ⶨʱЧ��
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));
		//ɨ�����������ͷŶ�������
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize) {
			Connection* p = _connectionQue.front();
			if (p->getAliveTime() > (_maxIdleTime * 1000)) {
				_connectionQue.pop();
				_connectionCnt--;
				delete p;//�ͷ����ӵ���~Connection()
			}
			else {
				break;//�����е�һ�����Ӳ������ͷ�������˵�����������Ҳ�����㣬�˳�ѭ��
			}
		}
	}
}

void CommonConnectionPool::produceConnectionTask() {
	for (;;) {
		unique_lock<mutex> lock(_queueMutex);
		//LOG("�������̳߳����������ӣ���ǰ���д�С: " + to_string(_connectionQue.size()) + ", ��ǰ������: " + to_string(_connectionCnt) + ", ���������: " + to_string(_maxSize));

		while (!_connectionQue.empty()) {
			//LOG("���Ӷ��в��գ������ߵȴ�...");
			_cv.wait(lock); // ���в���ʱ���ȴ�
		}

		// ����Ϊ�������ӳ�δ��ʱ�������µ�����
		if (_connectionCnt < _maxSize) {
			//LOG("����Ϊ�������ӳ�δ������ʼ�����µ�����");
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->RefreshAliveTime();
			_connectionQue.push(p);
			_connectionCnt++;
			//LOG("����������������ӵ����Ӷ��У���ǰ���д�С: " + to_string(_connectionQue.size()) + ", ��ǰ������: " + to_string(_connectionCnt));
		}
		else {
			//LOG("����Ϊ�յ����ӳ��������������µ�����");
		}

		_cv.notify_all(); // ֪ͨ������
		//LOG("�������߳�֪ͨ���еȴ���������");
	}
}

//�����ӳ���ȡ��һ������
shared_ptr<Connection> CommonConnectionPool::getConnection() {
	unique_lock<mutex> lock(_queueMutex);
	//LOG("���Ի�ȡ���ӣ���ǰ���д�С: " + to_string(_connectionQue.size()));

	while (_connectionQue.empty()) {
		//LOG("���Ӷ���Ϊ�գ��ȴ�����...");
		// wait_for()�����ȴ�ָ��ʱ�䣬ֱ���������㣬���߳�ʱ
		// cv_status::timeout��ʾ��ʱ
		if (cv_status::timeout == _cv.wait_for(lock, chrono::milliseconds(_connectionTimeout))) {
			// ����Ϊ��ʱ���ȴ�
			if (_connectionQue.empty()) {
				//LOG("��ȡ�������ӳ�ʱ...��ȡ����ʧ��");
				return nullptr; // ��ȡ�������ӳ�ʱ
			}
		}
	}
	//LOG("��ȡ�����ӣ�׼������");
	shared_ptr<Connection> sp(_connectionQue.front(), [&](Connection* pcon) {
		// �������ڷ�����Ӧ���߳��е��õģ����Ա��뱣֤�̰߳�ȫ
		unique_lock<mutex> lock(_queueMutex);
		_connectionQue.push(pcon);
		pcon->RefreshAliveTime();
		//LOG("���ӷ������ӳأ���ǰ���д�С: " + to_string(_connectionQue.size()));
		});
	_connectionQue.pop();
	//LOG("�����ѴӶ������Ƴ�����ǰ���д�С: " + to_string(_connectionQue.size()));

	// ˭���������һ�����ӣ���֪ͨ�����������µ�����
	if (_connectionQue.empty()) {
		//LOG("���Ӷ���Ϊ�գ�֪ͨ�����������µ�����");
		_cv.notify_all(); // ����Ϊ��ʱ��֪ͨ������
	}

	return sp;
}

/*
shared_ptr����ָ������ʱ�����Դֱ��delete��
�൱�ڵ���connection������������connection�ͱ�close���ˣ������ǷŻ����ӳ�
��Ҫ�Զ���shared_ptr���ͷŷ�ʽ����connectionֱ�ӹ黹���ӳ�
*/

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
		int idx = str.find("=", 0);
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

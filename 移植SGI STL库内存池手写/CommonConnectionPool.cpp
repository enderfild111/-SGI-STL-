#include "CommonConnectionPool.h"
//构造函数和析构函数
CommonConnectionPool::CommonConnectionPool()
{
	if (!loadConfigFile()) {
		LOG("配置文件加载失败");
		return;
	}
	/*
	创建初始数量的连接
	*/
	for (int i = 0; i < _initSize; i++) {
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->RefreshAliveTime();//每次进入队列，刷新空闲存活时间
		_connectionQue.push(p);
		_connectionCnt++;
		//cout << "创建第" << i + 1 << "个连接" << endl;
	}
	//启动一个新的线程，作为连接的生产者
	//利用绑定器，绑定一个对象的成员函数到一个线程上
	thread produce(std::bind(&CommonConnectionPool::produceConnectionTask, this));
	produce.detach();//线程分离，不阻塞主线程
	//启动一个新的定时线程，扫描超过_maxIdleTime的空闲连接，进行对于连接的回收
	thread scanner(std::bind(&CommonConnectionPool::scannerConnerctionTask, this));
	scanner.detach();//线程分离，不阻塞主线程
}

CommonConnectionPool::~CommonConnectionPool()
{

}
//线程安全的懒汉单例模式
CommonConnectionPool* CommonConnectionPool::getConnectionPool() {
	static CommonConnectionPool pool;//静态局部变量，编译器自动lock和unlock，线程安全
	return &pool;
} 
void CommonConnectionPool::scannerConnerctionTask() {
	for (;;) {
		//sleep模拟定时效果
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));
		//扫描整个队列释放多余连接
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize) {
			Connection* p = _connectionQue.front();
			if (p->getAliveTime() > (_maxIdleTime * 1000)) {
				_connectionQue.pop();
				_connectionCnt--;
				delete p;//释放连接调用~Connection()
			}
			else {
				break;//队列中第一个连接不满足释放条件，说明后面的连接也不满足，退出循环
			}
		}
	}
}

void CommonConnectionPool::produceConnectionTask() {
	for (;;) {
		unique_lock<mutex> lock(_queueMutex);
		//LOG("生产者线程尝试生产连接，当前队列大小: " + to_string(_connectionQue.size()) + ", 当前连接数: " + to_string(_connectionCnt) + ", 最大连接数: " + to_string(_maxSize));

		while (!_connectionQue.empty()) {
			//LOG("连接队列不空，生产者等待...");
			_cv.wait(lock); // 队列不空时，等待
		}

		// 队列为空且连接池未满时，生产新的连接
		if (_connectionCnt < _maxSize) {
			//LOG("队列为空且连接池未满，开始生产新的连接");
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->RefreshAliveTime();
			_connectionQue.push(p);
			_connectionCnt++;
			//LOG("新连接已生产并添加到连接队列，当前队列大小: " + to_string(_connectionQue.size()) + ", 当前连接数: " + to_string(_connectionCnt));
		}
		else {
			//LOG("队列为空但连接池已满，不生产新的连接");
		}

		_cv.notify_all(); // 通知消费者
		//LOG("生产者线程通知所有等待的消费者");
	}
}

//从连接池中取出一个连接
shared_ptr<Connection> CommonConnectionPool::getConnection() {
	unique_lock<mutex> lock(_queueMutex);
	//LOG("尝试获取连接，当前队列大小: " + to_string(_connectionQue.size()));

	while (_connectionQue.empty()) {
		//LOG("连接队列为空，等待连接...");
		// wait_for()函数等待指定时间，直到条件满足，或者超时
		// cv_status::timeout表示超时
		if (cv_status::timeout == _cv.wait_for(lock, chrono::milliseconds(_connectionTimeout))) {
			// 队列为空时，等待
			if (_connectionQue.empty()) {
				//LOG("获取空闲连接超时...获取连接失败");
				return nullptr; // 获取空闲连接超时
			}
		}
	}
	//LOG("获取到连接，准备返回");
	shared_ptr<Connection> sp(_connectionQue.front(), [&](Connection* pcon) {
		// 这里是在服务器应用线程中调用的，所以必须保证线程安全
		unique_lock<mutex> lock(_queueMutex);
		_connectionQue.push(pcon);
		pcon->RefreshAliveTime();
		//LOG("连接返回连接池，当前队列大小: " + to_string(_connectionQue.size()));
		});
	_connectionQue.pop();
	//LOG("连接已从队列中移除，当前队列大小: " + to_string(_connectionQue.size()));

	// 谁消费了最后一个连接，就通知生产者生产新的连接
	if (_connectionQue.empty()) {
		//LOG("连接队列为空，通知生产者生产新的连接");
		_cv.notify_all(); // 队列为空时，通知生产者
	}

	return sp;
}

/*
shared_ptr智能指针析构时会把资源直接delete，
相当于调用connection的析构函数，connection就被close掉了，而不是放回连接池
需要自定义shared_ptr的释放方式，把connection直接归还连接池
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
		if (idx == -1) {//空行或者注释行
			continue;
		}
		//解析key-value
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

#include "Connection.h"
#include"public.h"


Connection::Connection() : _alivetime(0) {
	_conn = mysql_init(nullptr);
}
Connection::~Connection() {
	
	if (_conn) {
		mysql_close(_conn);
	}
}

bool Connection::connect(std::string ip, unsigned short port,
	std::string username,
	std::string password,
	std::string dbname) {
	MYSQL* p = mysql_real_connect(_conn, ip.c_str(), username.c_str(),
		password.c_str(), dbname.c_str(), port, nullptr, 0);
	return p != nullptr;
}

bool Connection::update(std::string sql) {
	if (mysql_query(_conn, sql.c_str())) {
		LOG("����ʧ�ܣ�" + sql);
		std::cerr << "MySQL������Ϣ: " << mysql_error(_conn) << std::endl;
		return false;
	}
	return true;
}


MYSQL_RES* Connection::query(std::string sql) {
	if (mysql_query(_conn, sql.c_str())) {
		LOG("��ѯʧ�ܣ�" + sql);
		return nullptr;
	}
	return mysql_store_result(_conn);
}
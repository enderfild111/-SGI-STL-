#include<iostream>

#include"Connection.h"
#include"CommonConnectionPool.h"
using namespace std;
int main()
{
    clock_t start, end;
    start = clock();
    CommonConnectionPool* cp = CommonConnectionPool::getConnectionPool();
    for (int i = 0; i < 1000; ++i) {
        /*
        δʹ�����ӳ�
        */
      /*  Connection conn;
        char sql[1024] = {};
        sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhanshan", 20, "male");
        if (conn.connect("127.0.0.1", 3306, "root", "root", "chat")) {
            conn.update(sql);
        }
        else {
            std::cerr << "����ʧ��" << std::endl;
        }*/
        shared_ptr<Connection> sp = cp->getConnection();
        char sql[1024] = {};
        sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhanshan", 20, "male");
        sp->update(sql);
    }
    end = clock();
    cout << "�ܺ�ʱ��" << (double)(end - start) << "����" << endl;
    return 0;
}

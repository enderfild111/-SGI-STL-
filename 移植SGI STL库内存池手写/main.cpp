#include<iostream>

#include"Connection.h"

using namespace std;
int main()
{
    Connection conn;
    char sql[1024] = {};
    sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhanshan", 20, "male");
    if (conn.connect("127.0.0.1", 3306, "root", "root", "chat")) {
        conn.update(sql);
    }
    else {
        std::cerr << "Á¬½ÓÊ§°Ü" << std::endl;
    }
    return 0;
}

#include <iostream>
#include <mysql/mysql.h>

using namespace std;

int main()
{
    MYSQL mysql;
    MYSQL_RES *result;
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, "localhost", "liumingkai", "liumingkai", "test", 0, NULL, 0)) {
        cout << "connect mysql failed!" << endl;
        return -1;
    }
    mysql_query(&mysql, "select * from user;");

    result = mysql_store_result(&mysql);

    mysql_close(&mysql);
    return 0;
}

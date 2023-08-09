#include <cstdint>
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

    string sql = "select * from user;";
    mysql_real_query(&mysql, sql.c_str(), sql.length());

    result = mysql_store_result(&mysql);

    uint64_t raws = mysql_num_rows(result);
    if (raws == 0) {
        cout << sql << " . return null data" << endl;
    }

    cout << sql << " . return " << raws << "raws data." << endl;

    mysql_close(&mysql);
    return 0;
}

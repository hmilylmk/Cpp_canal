#include <cstdint>
#include <mysql/mysql.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include "mysqlConnector.h"

int MysqlConnector::connect() {
    if (!mysql_real_connect(&mysql_, host_.c_str(), username_.c_str(), password_.c_str(), db_.c_str(), 0, NULL, 0)) {
        std::cout << "connect mysql failed!" << std::endl;
        return -1;
    }
    return 0;
}

MysqlConnector::~MysqlConnector() {
    mysql_close(&mysql_);
}

std::unordered_map<std::string, std::string> MysqlConnector::getById(std::string &tableName, std::string &fields, int id) {
    std::string sql = "select " + fields + " from " + tableName + " where id = " + std::to_string(id);
    mysql_real_query(&mysql_, sql.c_str(), sql.size());

    MYSQL_RES *result;
    std::unordered_map<std::string, std::string> res;
    result = mysql_store_result(&mysql_);
    std::cout << sql << std::endl;
    uint64_t raws = mysql_num_rows(result);
    if (raws == 0) {
        return res;
    }

    MYSQL_FIELD *field = nullptr;
    MYSQL_ROW cur;
    while ((cur = mysql_fetch_row(result))) {
        ulong *lengths = mysql_fetch_lengths(result);
        mysql_field_seek(result, 0);
        for (uint off = 0; off < mysql_num_fields(result); off++) {
            const char *buffer;
            uint data_length;

            if (cur[off] == nullptr) {
                buffer = "NULL";
                data_length = 4;
            } else {
                buffer = cur[off];
                data_length = (uint)lengths[off];
            }
            field = mysql_fetch_field(result);
            std::string f(field->name, field->name_length);
            std::string v(buffer, data_length);
            res[f] = v;
        }
    }

    mysql_free_result(result);
    return res;
}

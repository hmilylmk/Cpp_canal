#ifndef MYSQLCONNECTOR_H_BZAPGKIO
#define MYSQLCONNECTOR_H_BZAPGKIO

#include <string>
#include <mysql/mysql.h>

class MysqlConnector {
    MysqlConnector(std::string &host, std::string &username, std::string &password, std::string &db) : host_(host), username_(username), password_(password), db_(db) {
        mysql_init(&mysql_);
    }

    int connect();

    int select(std::string &sqlSelect);
    int update(std::string &sqlUpdate);

private:
    MYSQL mysql_;
    std::string host_;
    std::string username_;
    std::string password_;
    std::string db_;
};

#endif /* end of include guard: MYSQLCONNECTOR_H_BZAPGKIO */

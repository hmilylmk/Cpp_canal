#include <hiredis/hiredis.h>
#include <iostream>
#include <unordered_map>
#include "mysqlConnector.h"
#include "redisConnector.h"

using namespace std;

int main()
{
    std::string hostname = "127.0.0.1";
    int port = 6379;
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    std::string host = "localhost";
    std::string name = "liumingkai";
    std::string passwd = "liumingkai";
    std::string db = "test";
    std::string table = "user";
    std::string fields = "id, name, password";

    MysqlConnector mc(host, name, passwd, db);
    mc.connect();

    std::unordered_map<std::string, std::string> re = mc.getById(table, fields, 12345);
    if (re.empty()) {
        std::cout << "none data" << std::endl;
    }
    else {
        for (auto i : re) {
            std::cout << i.first << ", " << i.second << std::endl;
        }
    }

    RedisConnector rc(hostname, port, timeout);
    rc.init();
    redisReply *rr;

    rr = static_cast<redisReply*>(redisCommand(rc.getRedis(), "HGETALL 1111"));

    if (rr->type == REDIS_REPLY_ARRAY) {
        for (int j = 0; j < rr->elements; j++) {
            printf("%u) %s\n", j, rr->element[j]->str);
        }
    }
    std::cout << rr->type << std::endl;
    freeReplyObject(rr);

    return 0;

}

#ifndef REDISCONNECTOR_H_ICBKP1LU
#define REDISCONNECTOR_H_ICBKP1LU

#include <string>
#include <bits/types/struct_timeval.h>
#include <hiredis/hiredis.h>

class RedisConnector {
public:
    RedisConnector(const std::string &hostname, const int port, const timeval &timeout) : port_(port), hostname_(hostname), timeout_(timeout), redis_(nullptr) {};
    ~RedisConnector();
    int init();
    void redisCmd(std::string cmd);

    redisContext* getRedis() const;

private:
    int port_;
    std::string hostname_;
    timeval timeout_;
    redisContext *redis_;
};

#endif /* end of include guard: REDISCONNECTOR_H_ICBKP1LU */

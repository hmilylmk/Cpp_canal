#include "redisConnector.h"

int RedisConnector::init() {
    redis_ = redisConnectWithTimeout(hostname_.c_str(), port_, timeout_);
    if (redis_ == nullptr || redis_->err) {
        if (redis_) {
            printf("Connection error: %s\n", redis_->errstr);
            redisFree(redis_);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        return -1;
    }
    return 0;
}

void RedisConnector::redisCmd(std::string cmd) {

}

redisContext* RedisConnector::getRedis() const {
    return redis_;
}

RedisConnector::~RedisConnector() {
    redisFree(redis_);
}

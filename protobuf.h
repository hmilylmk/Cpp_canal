#include <memory>
#include <string>
#include <vector>
#include "CanalProtocol.pb.h"
#include "EntryProtocol.pb.h"

class Connector {
public:
    Connector(const std::string &address = "127.0.0.1", int port = 1111) : address_(address), port_(port) {};
    int connect();
    std::string read(int len);
    std::string readNextPacket();
    int write(void *buf, int len);
    int writeWithHead(const std::string &data);
private:
    std::string address_;
    int port_;
    int socket_;
};

class Client {
public:
    Client(const std::string address, int port) : conn_(address, port) {}
    int connect(); 
    bool checkValid(const std::string username, const std::string password);
    bool subscribe(const std::string &clientId = "1001", const std::string &destination = "example", const std::string &filter = ".*\\..*");
    std::vector<com::alibaba::otter::canal::protocol::Entry> get(int size = 100);
    std::vector<com::alibaba::otter::canal::protocol::Entry> getWithoutAck(int batchSize, int timeout, int unit, int *batchId);
    int ack(int messagesId);
    int rollback(int batchId);
private:
    Connector conn_;
    std::string clientId_;
    std::string destination_;
};

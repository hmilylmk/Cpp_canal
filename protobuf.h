#include <memory>
#include <string>

class Connector {
public:
    Connector(std::string address = "127.0.0.1", int port = 1111) : address_(address), port_(port) {};
    int connect();
    std::string read(int len);
    std::string readNextPacket();
    int write(const std::string& data);
    int writeWithHead(const std::string &data);
private:
    static const int maxReadLen = 1024;
    std::string address_;
    int port_;
    int socket_;
};

class Client {
public:
    Client(std::string &address, int port) : conn_(address, port) {}
    int connect(); 
    bool checkValid(const std::string &username, const std::string &password);
    bool subscribe(const std::string &clientId, const std::string &destination, const std::string &filter);
    int get(int size);
    int getWithoutAck();
private:
    Connector conn_;
    std::string clientId_;
    std::string destination_;
};

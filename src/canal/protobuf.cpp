#include <ios>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "CanalProtocol.pb.h"
#include "protobuf.h"


int Connector::connect() {
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port_);
    sa.sin_addr.s_addr = inet_addr(address_.c_str());

    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ == -1) {
        std::cout << "socket error!" << std::endl;
    }

    int ret = ::connect(socket_, (sockaddr *)&sa, sizeof(sa));
    if (ret == -1) {
        std::cout << "connect error!" << std::endl;
        return -1;
    }

    std::cout << "connect successfully" << std::endl;;
    return 0;
}

void* Connector::read(int len) {
    char *buf = new char[len + 1];
    int receive = ::recv(socket_, buf, len, MSG_WAITALL);
    if (receive != len) {
        std::cout << "recv error" << std::endl;
    }
    buf[len] = '\0';
    return static_cast<void *>(buf);
}

std::string Connector::readNextPacket() {
    void *buf = read(4);
    unsigned int packetLen = ::ntohl(*static_cast<unsigned int *>(buf));
    delete static_cast<char *>(buf);
    // string store binary data, need construct with len parameter
    // otherwise will be cut off
    return std::string(static_cast<char *>(read(packetLen)), packetLen);
}

int Connector::write(void *buf, int len) {
    return ::send(socket_, buf, len, 0);
}

int Connector::writeWithHead(const std::string &data) {
    unsigned int len = data.length();
    unsigned int endianLen = ::htonl(len);
    void *buf = const_cast<char*>(data.c_str());
    write(static_cast<void*>(&endianLen), 4);
    return write(buf, len);
}

int Client::connect() {
    int ret = conn_.connect();
    if (ret != 0) {
        return -1;
    }
    std::string data = conn_.readNextPacket();
    com::alibaba::otter::canal::protocol::Packet packet;
    packet.ParseFromString(data);
    if (packet.type() != com::alibaba::otter::canal::protocol::PacketType::HANDSHAKE) {
        std::cout << "Client connect error!" << std::endl;
        return -1;
    }
    return 0;
}

bool Client::checkValid(const std::string username, const std::string password) {
    com::alibaba::otter::canal::protocol::ClientAuth clientAuth;
    clientAuth.set_username(username);
    clientAuth.set_password(password);
    std::string serialBody;
    clientAuth.SerializeToString(&serialBody);

    com::alibaba::otter::canal::protocol::Packet packetWrite;
    packetWrite.set_type(com::alibaba::otter::canal::protocol::PacketType::CLIENTAUTHENTICATION);
    packetWrite.set_body(serialBody);
    std::string serial;
    packetWrite.SerializeToString(&serial);
    conn_.writeWithHead(serial);
    std::string data = conn_.readNextPacket();
    com::alibaba::otter::canal::protocol::Packet packetRead;
    packetRead.ParseFromString(data);
    if (packetRead.type() != com::alibaba::otter::canal::protocol::PacketType::ACK) {
        std::cout << "Client auth error!" << std::endl;
        return false;
    }
    com::alibaba::otter::canal::protocol::Ack ack;
    ack.ParseFromString(packetRead.body());
    if (ack.error_code() > 0) {
        std::cout << "auth failed, code: " << ack.error_code() << ", message: " << ack.error_message() << std::endl;
        return false;
    }
    std::cout << "auth successfully!" << std::endl;
    return true;
}

bool Client::subscribe(const std::string &clientId, const std::string &destination, const std::string &filter) {
    clientId_ = clientId;
    destination_ = destination;
    rollback(0);
    com::alibaba::otter::canal::protocol::Sub sub;
    sub.set_client_id(clientId);
    sub.set_destination(destination);
    sub.set_filter(filter);
    std::string subS;
    sub.SerializeToString(&subS);
    com::alibaba::otter::canal::protocol::Packet packetWrite;
    packetWrite.set_type(com::alibaba::otter::canal::protocol::PacketType::SUBSCRIPTION);
    packetWrite.set_body(subS);
    std::string serial;
    packetWrite.SerializeToString(&serial);
    conn_.writeWithHead(serial);

    com::alibaba::otter::canal::protocol::Packet packetRead;
    std::string data = conn_.readNextPacket();
    packetRead.ParseFromString(data);
    std::cout << packetRead.type() << std::endl;
    com::alibaba::otter::canal::protocol::Ack ack;
    ack.ParseFromString(packetRead.body());
    if (ack.error_code() > 0) {
        std::cout << "subscribe failed, code: " << ack.error_code() << ", message: " << ack.error_message() << std::endl;
        return false;
    }
    return true;
}

int Client::ack(int messagesId) {
    if (messagesId) {
        com::alibaba::otter::canal::protocol::ClientAck cack;
        cack.set_destination(destination_);
        cack.set_client_id(clientId_);
        cack.set_batch_id(messagesId);
        std::string serialBody;
        cack.SerializeToString(&serialBody);

        com::alibaba::otter::canal::protocol::Packet packetWrite;
        packetWrite.set_type(com::alibaba::otter::canal::protocol::PacketType::CLIENTACK);
        packetWrite.set_body(serialBody);
        std::string data;
        packetWrite.SerializeToString(&data);
        conn_.writeWithHead(data);
    }
    return 0;
}

int Client::rollback(int batchId) {
    com::alibaba::otter::canal::protocol::ClientRollback crb;
    crb.set_destination(destination_);
    crb.set_client_id(clientId_);
    crb.set_batch_id(batchId);
    std::string serialBody;
    crb.SerializeToString(&serialBody);

    com::alibaba::otter::canal::protocol::Packet packet;
    packet.set_type(com::alibaba::otter::canal::protocol::PacketType::CLIENTROLLBACK);
    packet.set_body(serialBody);
    std::string data;

    packet.SerializeToString(&data);

    conn_.writeWithHead(data);
    return 0;
}

std::vector<com::alibaba::otter::canal::protocol::Entry> Client::get(int size) {
    int batchId;
    std::vector<com::alibaba::otter::canal::protocol::Entry> messages = getWithoutAck(size, -1, -1, &batchId);
    ack(batchId);
    return messages;
}

std::vector<com::alibaba::otter::canal::protocol::Entry> Client::getWithoutAck(int batchSize, int timeout, int unit, int *batchId) {
    com::alibaba::otter::canal::protocol::Get getp;
    getp.set_client_id(clientId_);
    getp.set_destination(destination_);
    getp.set_auto_ack(false);
    getp.set_timeout(timeout);
    getp.set_fetch_size(batchSize);
    getp.set_unit(unit);
    std::string serialBody;
    getp.SerializeToString(&serialBody);

    com::alibaba::otter::canal::protocol::Packet packetWrite;
    packetWrite.set_type(com::alibaba::otter::canal::protocol::PacketType::GET);
    packetWrite.set_body(serialBody);

    std::string serial;
    packetWrite.SerializeToString(&serial);
    conn_.writeWithHead(serial);

    com::alibaba::otter::canal::protocol::Packet packetRead;
    std::string data = conn_.readNextPacket();
    packetRead.ParseFromString(data);
    std::vector<com::alibaba::otter::canal::protocol::Entry> res;
    if (packetRead.type() == com::alibaba::otter::canal::protocol::PacketType::MESSAGES) {
        com::alibaba::otter::canal::protocol::Messages messages;
        messages.ParseFromString(packetRead.body());
        if (messages.batch_id() > 0) {
            *batchId = messages.batch_id();
            const ::google::protobuf::RepeatedPtrField<std::string>& me = messages.messages();
            for (auto it = me.begin(); it != me.end(); ++it) {
                com::alibaba::otter::canal::protocol::Entry e;
                e.ParseFromString(*it);
                res.push_back(e);
            }
        }
    }
    return res;
}

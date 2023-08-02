#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <iostream>
#include "protobuf.h"
#include "CanalProtocol.pb.h"

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

std::string Connector::read(int len) {
    char *buf = new char[len];
    ::recv(socket_, buf, len, 0);
    return std::string(buf);
}

std::string Connector::readNextPacket() {
    std::string len = read(4);
    void *buf = const_cast<char*>(len.c_str());
    unsigned int packetLen = ::ntohl(*static_cast<unsigned int *>(buf));
    return read(packetLen);
}

int Connector::write(void *buf, int len) {
    return ::send(socket_, buf, len, 0);
}

int Connector::writeWithHead(const std::string &data) {
    unsigned int len = data.length();
    unsigned int endianLen = ::htonl(len);
    void *buf = const_cast<char*>(data.c_str());
    write((void*)&endianLen, 4);
    write(buf, len);
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
    }
}

bool Client::checkValid(const std::string &username, const std::string &password) {
    com::alibaba::otter::canal::protocol::ClientAuth clientAuth;
    clientAuth.set_username(username);
    clientAuth.set_password(password);
    string serialBody;
    clientAuth.SerializeToString(&serialBody);

    com::alibaba::otter::canal::protocol::Packet packetWrite;
    packetWrite.set_type(com::alibaba::otter::canal::protocol::PacketType::CLIENTAUTHENTICATION);
    packetWrite.set_body(serialBody);
    string serial;
    packetWrite.SerializeToString(&serial);
    conn_.writeWithHead(serial);
    std::string data = conn_.readNextPacket();
    com::alibaba::otter::canal::protocol::Packet packetRead;
    packetRead.ParseFromString(data);
    if (packetRead.type() != com::alibaba::otter::canal::protocol::PacketType::ACK) {
        std::cout << "Client auth error!" << std::endl;
        reture false;
    }
    com::alibaba::otter::canal::protocol::Ack ack;
    ack.ParseFromString(packetRead.body());
    if (ack.error_code() > 0) {
        std::cout << "auth failed, code: " << ack.error_code() << ", message: " << ack.error_message() << endl;
        return false;
    }
    std::cout << "auth successfully!" << endl;
    return true;
}

bool Client::subscribe(const std::string &clientId, const std::string &destination, const std::string &filter) {
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
    if (packetRead.type() != com::alibaba::otter::canal::protocol::PacketType::ACK) {
        std::cout << "Client subscrib error!" << std::endl;
        reture false;
    }
    com::alibaba::otter::canal::protocol::Ack ack;
    ack.ParseFromString(packetRead.body());
    if (ack.error_code() > 0) {
        std::cout << "subscribe failed, code: " << ack.error_code() << ", message: " << ack.error_message() << endl;
        return false;
    }
    return true;
}

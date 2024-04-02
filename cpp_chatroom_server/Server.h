#ifndef CPP_CHATROOM_SERVER_SERVER_H
#define CPP_CHATROOM_SERVER_SERVER_H

#include <mutex>
#include <string>
#include <utility>
#include <queue>
#include <condition_variable>

#include "NetworkEntity.h"
#include "dataStructure.h"

using std::string;
using std::queue;
using std::mutex;
using std::pair;
using std::condition_variable;
using std::lock_guard;

class Server {
public:
    SSL *ssl;
    int client_sockfd;
    string ClientName;

    Server();

    ~Server();

    int waitingClientConnect(SSL_CTX *ssl_ctx, int sockfd);

    Message *receiveMessage();

    void sendMessage(Message *);
};

Server::Server() {}

Server::~Server() {}

int Server::waitingClientConnect(SSL_CTX *ssl_ctx, int sockfd) {
    // 等待客户端连接
    client_sockfd = accept(sockfd, nullptr, nullptr);
    if (client_sockfd < 0) {
        LOG(WARNING) << "Accepting connection failed.";
        exit(EXIT_FAILURE);
    }

    // 创建SSL对象并将其与客户端套接字关联
    ssl = SSL_new(ssl_ctx);
    SSL_set_fd(ssl, client_sockfd);

    // 开始SSL握手
    if (SSL_accept(ssl) <= 0) {
        LOG(WARNING) << "SSL handshake failed.";
        //在握手失败后检查错误队列
        long error;
        while ((error = ERR_get_error()) != 0) {
            char error_buffer[256];
            ERR_error_string(error, error_buffer);
            LOG(WARNING) << "SSL error: " << error_buffer;
        }
        exit(EXIT_FAILURE);
    }
    Message *HelloMsg = receiveMessage();
    ClientName = HelloMsg->retSender();
    LOG(INFO) << ClientName << " join the ChatRoom.";
    return client_sockfd;
}

Message *Server::receiveMessage() {
    char buffer[BUFFER_SIZE];
    int bytes_received = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (bytes_received <= 0) {
        LOG(INFO) << "Failed to receive data from client [" << client_sockfd << "], closing connection.";
        return nullptr;
    }
    buffer[bytes_received] = '\0';

    Message *message = new Message;
    message->decode(buffer);
    return message;
}

void Server::sendMessage(Message *message) {
    string buffer = message->encode();
    SSL_write(ssl, buffer.c_str(), buffer.size());
}

#endif //CPP_CHATROOM_SERVER_SERVER_H

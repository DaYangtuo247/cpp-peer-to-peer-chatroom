#ifndef CPP_CHATROOM_Client_Client_H
#define CPP_CHATROOM_Client_Client_H

#include <string>

#include "NetworkEntity.h"
#include "dataStructure.h"

using std::cin;
using std::string;

class Client {
public:
    SSL *ssl;
    string ClientName;
    string rsaPublicKey;
    string aesKey;

    Client();

    ~Client();

    void connectServer(SSL_CTX *ssl_ctx, int sockfd, struct sockaddr_in server_addr);

    void receiveMessage();

    void sendMessage(Message *message);

    void sendComment();
};

Client::Client() {}

Client::~Client() {
    // 关闭SSL连接
    SSL_shutdown(ssl);
    SSL_free(ssl);
}

void Client::connectServer(SSL_CTX *ssl_ctx, int sockfd, struct sockaddr_in server_addr) {
    // 连接服务器
    if (connect(sockfd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
        LOG(WARNING) << "Connection failed.";
        exit(EXIT_FAILURE);
    }

    // 创建SSL对象并将其与套接字关联
    ssl = SSL_new(ssl_ctx);
    SSL_set_fd(ssl, sockfd);

    // 开始SSL握手
    if (SSL_connect(ssl) <= 0) {
        LOG(WARNING) << "SSL handshake failed.";
        // 在握手失败后检查错误队列
        long error;
        while ((error = ERR_get_error()) != 0) {
            char error_buffer[256];
            ERR_error_string(error, error_buffer);
            LOG(WARNING) << "SSL error: " << error_buffer;
        }
        exit(EXIT_FAILURE);
    }
    LOG(INFO) << "connect Server success.";


    LOG(INFO) << "Server: input you name.";
    cin >> ClientName;
    Message message;
    message.add_sender_receiver(ClientName, "Server");
    string buffer = message.encode();
    SSL_write(ssl, buffer.c_str(), buffer.size());
}

void Client::receiveMessage() {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        if (bytes_received <= 0) {
            LOG(WARNING) << "Failed to receive data.";
            exit(EXIT_FAILURE);
        }
        // 添加字符串结束符，打印接收到的数据
        buffer[bytes_received] = '\0';

        Message *message = new Message;
        message->decode(buffer);
        LOG(INFO) << message->retSender() << ": " << message->retMessage();
    }
}

void Client::sendMessage(Message *message) {
    string buffer = message->encode();
    SSL_write(ssl, buffer.c_str(), buffer.size());
}

// 控制输入
void Client::sendComment() {
    while (true) {
        string input;
        cin >> input;
        Message *message = new Message;
        message->json_init("chatroom", "message");
        message->add_sender_receiver(ClientName, "Server");
        message->add_message(input);
        sendMessage(message);
    }
}

#endif //CPP_CHATROOM_Client_Client_H

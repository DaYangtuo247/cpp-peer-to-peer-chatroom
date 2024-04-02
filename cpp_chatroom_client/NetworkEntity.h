#ifndef CPP_CHATROOM_SERVER_NETWORKENTITY_H
#define CPP_CHATROOM_SERVER_NETWORKENTITY_H

// 确定是服务器还是客户端，便于如下函数的选择
#define USE_OBJECT "Client"
// 确定缓冲区大小
#define BUFFER_SIZE 2048

#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <winsock2.h>

// 加载日志系统
#include "include/easyloggingpp/easylogging++.h"
#include "include/easyloggingpp/easylogging++.cc"

INITIALIZE_EASYLOGGINGPP

// openssl 证书
#define CERT_FILE "server.crt"
#define KEY_FILE "server.key"

using std::endl;

class NetworkEntity {
public:
    SSL_CTX *ssl_ctx;
    int sockfd;
    struct sockaddr_in server_addr;

    NetworkEntity(const char *, int);

    ~NetworkEntity();

    void initWindowsLib();

    // 如下两个个函数是Server使用
    void initOpensslServer();

    void initServerSocket();
};

NetworkEntity::NetworkEntity(const char *DEFAULT_IP, int DEFAULT_PORT) {
    // 初始化winsock库
    initWindowsLib();

    // 初始化OpenSSL
    SSL_library_init();
    if(!strcmp(USE_OBJECT, "Server")){
        ssl_ctx = SSL_CTX_new(SSLv23_server_method());
    } else {
        ssl_ctx = SSL_CTX_new(SSLv23_client_method());
    }
    if (!ssl_ctx) {
        LOG(WARNING) << "SSL context creation failed.";
        exit(EXIT_FAILURE);
    }

    if(!strcmp(USE_OBJECT, "Server")){
        initOpensslServer();
    }

    // 创建TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG(WARNING) << "socket creation error";
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_IP);
    server_addr.sin_port = htons(DEFAULT_PORT);

    if(!strcmp(USE_OBJECT, "Server")){
        initServerSocket();
    }
}

NetworkEntity::~NetworkEntity() {
    // 关闭服务器套接字
    closesocket(sockfd);
    // 清理SSL上下文
    SSL_CTX_free(ssl_ctx);
    // 关闭windows库
    WSACleanup();
}

void NetworkEntity::initWindowsLib() {
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        LOG(WARNING) << "winsock error: " << err;
        exit(EXIT_FAILURE);
    }
}

void NetworkEntity::initOpensslServer() {
    // 加载证书和私钥
    if (SSL_CTX_use_certificate_file(ssl_ctx, CERT_FILE, SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ssl_ctx, KEY_FILE, SSL_FILETYPE_PEM) <= 0) {
        LOG(WARNING) << "Error loading certificate or private key.";
        exit(EXIT_FAILURE);
    }
    //验证私钥是否有效
    if (!SSL_CTX_check_private_key(ssl_ctx)) {
        LOG(WARNING) << "private key error";
        exit(EXIT_FAILURE);
    }
}

void NetworkEntity::initServerSocket() {
    // 绑定ip与端口
    if (bind(sockfd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
        LOG(WARNING) << "Binding failed.";
        exit(EXIT_FAILURE);
    }

    // 监听 ip:port
    LOG(INFO) << "The server is started, listening on port 5270.";
    listen(sockfd, 20);
}

#endif //CPP_CHATROOM_SERVER_NETWORKENTITY_H

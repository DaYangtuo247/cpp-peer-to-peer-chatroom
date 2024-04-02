#include "Server.h"
#include "PacketForwarding.h"

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 5270

int main() {
    // 加载日志系统
    el::Configurations conf("server_log.conf");
    el::Loggers::reconfigureAllLoggers(conf);

    // 启动socket、openssl、监听ip:port
    NetworkEntity net(DEFAULT_IP, DEFAULT_PORT);

    // 创建同步进程处理消息队列
    PackerForwarding server_manage;
    // 创建消息队列管理子线程
    thread processorThread(&PackerForwarding::epoll_wait_thread, &server_manage);

    Server server;

    // 为每一个连接创建一个server对象
    while (true) {
        int client_sockfd = server.waitingClientConnect(net.ssl_ctx, net.sockfd);
        epoll_push(client_sockfd);
    }
    return 0;
}
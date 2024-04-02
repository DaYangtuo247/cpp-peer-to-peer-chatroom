#include <thread>
#include "Client.h"

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 5270
using std::cin;
using std::thread;


int main() {
    // 加载日志系统
    el::Configurations conf("log.conf");
    el::Loggers::reconfigureAllLoggers(conf);

    // 启动socket、openssl
    NetworkEntity net(DEFAULT_IP, DEFAULT_PORT);

    // 创建一个Client对象
    Client *client = new Client();
    client->connectServer(net.ssl_ctx, net.sockfd, net.server_addr);

    Sleep(1000);
    // 创建发送接收线程
    thread(&Client::receiveMessage, client).detach();
    thread(&Client::sendComment, client).detach();

    return 0;
}

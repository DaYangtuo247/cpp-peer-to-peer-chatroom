#ifndef CPP_CHATROOM_SERVER_PACKETFORWARDING_H
#define CPP_CHATROOM_SERVER_PACKETFORWARDING_H

#include <thread>

using std::thread;
using std::unordered_map;
using std::unique_lock;
using namespace std;


class PackerForwarding {
public:
    // 历史消息记录
    vector<Message> record;
    // 消息队列
    queue<pair<int, Message *>> message_que;
    // 使用互斥锁保护对 消息队列 的访问
    mutex message_que_mutex;


    // 定义 client_sockfd 和 Server 对象的映射
    unordered_map<int, Server *> clientSockfd_Server_map;
    // 定义 reveiver 和 client_sockfd 的映射
    unordered_map<string, int> clientName_clientSockfd_map;
    // 使用互斥锁保护 clientSockfd_Server_map 以及 clientName_clientSockfd_map 的访问
    mutex clientSockfd_mutex;

    // 同步原语，控制 message_que_Processor 函数的阻塞与唤醒
    condition_variable start_message_que_processor;

    PackerForwarding();

    ~PackerForwarding();

    void router(int client_sockfd, Message *message);

    void message_que_Processor();

    void start_receive_thread(int client_sockfd);

    void welcomeClient(int client_sockfd, Server *server);
};

PackerForwarding::PackerForwarding() {}

PackerForwarding::~PackerForwarding() {}

// 路由，选择转发对象
void PackerForwarding::router(int client_sockfd, Message *message) {
    LOG(INFO) << "router start: " << client_sockfd;
    // 端对端传输
    if (message->retType() == string("E2EE")) {
        int target_client_sockfd = clientName_clientSockfd_map[message->retReceiver()];
        clientSockfd_Server_map[target_client_sockfd]->sendMessage(message);
        LOG(INFO) << "[" << message->retSender() << "] --> [" << message->retReceiver() << "]: "
                  << message->retMessage();
    }
        //聊天室
    else if (message->retType() == string("chatroom")) {
        for (auto &[sockfd, server]: clientSockfd_Server_map) {
            if (sockfd == client_sockfd)
                continue;
            LOG(INFO) << "[" << message->retSender() << "]: " << message->retMessage();
            server->sendMessage(message);
        }
    }
}

// 同步进程，当 message_que_Processor 检测到 message_que非空时，
// 他将唤醒 message_que_Processor 函数, 否则阻塞 message_que_Processor
void PackerForwarding::message_que_Processor() {
    while (true) {
        LOG(INFO) << "message_que_Processor";
        unique_lock<mutex> lock(message_que_mutex);
        // 当发现message_que为空时，阻塞 message_que_Processor 函数
        start_message_que_processor.wait(lock, [&] {
            return !message_que.empty();
        });

        // 消息分发完成在解锁
        while (!message_que.empty()) {
            pair<int, Message *> message = message_que.front();
            message_que.pop();
            router(message.first, message.second);
        }

        // 解锁互斥锁，以允许其他线程访问 message_que
        lock.unlock();
    }
}

// 对 client_sockfd 的接收线程
void PackerForwarding::start_receive_thread(int client_sockfd) {
    Message *message = nullptr;
    // 不断接受来自 client_sockfd 的消息
    do {
        message = clientSockfd_Server_map[client_sockfd]->receiveMessage();
        LOG(INFO) << "[" << client_sockfd << "]: " << message->retMessage();
        // 使用互斥锁保护 message_que 的访问
        {
            lock_guard<mutex> lock(message_que_mutex);
            message_que.push({client_sockfd, message});
        }
        // 唤醒 message_que_Processor 函数
        start_message_que_processor.notify_one();
    } while (message != nullptr);
    // 关闭客户端套接字并从映射中移除
    LOG(INFO) << "delete [" << client_sockfd << "] record.";
    closesocket(client_sockfd);
    delete clientSockfd_Server_map[client_sockfd];
    clientSockfd_Server_map.erase(client_sockfd);
}


// 创建线程处理客户端链接，同时向客户端发送消息
void PackerForwarding::welcomeClient(int client_sockfd, Server *server) {
    LOG(INFO) << "[" << server->ClientName << "] Client connect the Server. online ["
              << clientSockfd_Server_map.size() << "] people.";
    Message message;
    message.json_init("chatroom", "message");
    message.add_sender_receiver("Server", "You");
    message.add_message("welcome us the ChatRoom. input you name.");

    {
        lock_guard<mutex> lock(clientSockfd_mutex);
        clientSockfd_Server_map.emplace(client_sockfd, server);
    }

    clientSockfd_Server_map[client_sockfd]->sendMessage(&message);
}

#endif //CPP_CHATROOM_SERVER_PACKETFORWARDING_H

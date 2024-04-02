/*
    {
        "type": "E2EE/chatroom/protocol",
        "format": "message/file",
        data : {
            "name": "file_name",
            "content": "message content or file data",
        },
        "sender": "sender_username",
        "receiver": "receiver_username",
        "timestamp": "YYYY-MM-DD HH:MM:SS",
        // 如果使用chatroom，那么没有sender_public_key、aes_encrypt
        "sender_public_key": "receiver_public_key",
        "aes_encrypt": "aes_encrypt"
    }
 */

#ifndef CPP_CHATROOM_SERVER_DATASTRUCTURE_H
#define CPP_CHATROOM_SERVER_DATASTRUCTURE_H

#include "include/nlohmann/json.hpp"
#include <string>

using std::string;
using JSON = nlohmann::json;

class Message {
private:
    JSON json;
public:
    void json_init(string type, string format);

    void add_sender_receiver(string sender, string receiver);

    void add_pulicKey_aesEncryptKey(string publicKey, string aesEncryptKey);

    void add_message(string message);

    void add_file(string file_name, FILE *filefd);

    string encode();

    void decode(char *);

    string retMessage();

    string retType();

    string retFormat();

    string retSender();
    string retReceiver();
};

void Message::json_init(string type, string format) {
    json["type"] = type;
    json["format"] = format;
    json["data"] = {{"name",    nullptr},
                    {"content", nullptr}};
    json["sender"] = nullptr;
    json["receiver"] = nullptr;
    json["timestamp"] = nullptr;
}

void Message::add_sender_receiver(string sender, string receiver) {
    json["sender"] = sender;
    json["reveiver"] = receiver;
}

void Message::add_pulicKey_aesEncryptKey(string publicKey, string aesEncryptKey) {
    json["publicKey"] = publicKey;
    json["aesEncryptKey"] = aesEncryptKey;
}

void Message::add_message(string message) {
    json["data"]["content"] = message;
}

void Message::add_file(string file_name, FILE *filefd) {
    json["data"]["name"] = file_name;
    // 关于传输文件，暂时没写
}

string Message::encode() {
    return json.dump();
}

void Message::decode(char *buffer) {
    json = JSON::parse(buffer);
}

string Message::retMessage() {
    return json["data"]["content"];
}

string Message::retSender() {
    return json["sender"];
}

string Message::retReceiver() {
    return json["reveiver"];
}

string Message::retFormat() {
    return json["format"];
}

string Message::retType() {
    return json["type"];
}

#endif //CPP_CHATROOM_SERVER_DATASTRUCTURE_H

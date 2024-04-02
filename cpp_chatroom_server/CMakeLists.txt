cmake_minimum_required(VERSION 3.27)
project(cpp_chatroom_server)

set(CMAKE_CXX_STANDARD 17)

# 使头文件可访问
include_directories("C:/Program Files/OpenSSL-Win64/include")
# 指定动态库路径
link_directories("C:/Program Files/OpenSSL-Win64/bin")

add_executable(cpp_chatroom_server
        main.cpp
        NetworkEntity.h
        dataStructure.h
        Server.h
        ../cpp_chatroom_server/include/easyloggingpp/easylogging++.h
        PacketForwarding.h
)

target_link_libraries(cpp_chatroom_server libssl-3-x64 libcrypto-3-x64 ws2_32)
# 静态链接ddl库
#set(CMAKE_EXE_LINKER_FLAGS -static)
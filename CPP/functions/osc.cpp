#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <functional>

#pragma comment(lib, "ws2_32.lib")

// 发送 OSC 消息
void sendOSC(const std::string& ip, int port, const std::string& address, const std::string& msgText, bool flag) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

    std::vector<char> msg;

    msg.insert(msg.end(), address.begin(), address.end());
    msg.push_back('\0');
    while (msg.size() % 4) msg.push_back('\0');

    msg.push_back(',');
    msg.push_back('s');
    msg.push_back(flag ? 'T' : 'F');
    while (msg.size() % 4) msg.push_back('\0');

    msg.insert(msg.end(), msgText.begin(), msgText.end());
    msg.push_back('\0');
    while (msg.size() % 4) msg.push_back('\0');

    sendto(sock, msg.data(), msg.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    closesocket(sock);
    WSACleanup();
}

// AFK 回调类型
using AFKCallback = std::function<void(bool)>;

// 启动 AFK 监听
void startAFKListener(int listenPort, AFKCallback callback) {
    std::thread([listenPort, callback]() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) return;

        SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET) return;

        sockaddr_in localAddr{};
        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons(listenPort);
        localAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) return;

        char buffer[1024];
        while (true) {
            int len = recv(sock, buffer, sizeof(buffer)-1, 0);
            if (len > 0) {
                buffer[len] = '\0';
                std::string msg(buffer);

                if (msg.find("/avatar/parameters/AFK") != std::string::npos) {
                    bool afk = msg.find('T') != std::string::npos;
                    callback(afk);
                }
            }
        }

        closesocket(sock);
        WSACleanup();
    }).detach();
}
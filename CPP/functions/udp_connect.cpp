#include "udp_connect.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

UDPClient::UDPClient(const std::string& ip, int port)
    : serverIP(ip), serverPort(port), sock(nullptr)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        throw std::runtime_error("WSAStartup failed");

    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("Failed to create socket");
    }
    sock = (void*)s;
}

UDPClient::~UDPClient() {
    if (sock) {
        closesocket((SOCKET)sock);
        WSACleanup();
    }
}

// 简单手动解析 JSON 风格字符串 {"battery":100,"charge_state":true}
static BatteryInfo parseBatteryResponse(const std::string& msg) {
    BatteryInfo info{0,false};
    auto findInt = [&](const std::string& key){
        size_t pos = msg.find(key);
        if (pos != std::string::npos) {
            pos = msg.find(":", pos);
            if (pos != std::string::npos) {
                size_t end = msg.find_first_of(",}", pos);
                return std::stoi(msg.substr(pos+1, end-pos-1));
            }
        }
        return 0;
    };
    auto findBool = [&](const std::string& key){
        size_t pos = msg.find(key);
        if (pos != std::string::npos) {
            pos = msg.find(":", pos);
            if (pos != std::string::npos) {
                size_t end = msg.find_first_of(",}", pos);
                std::string val = msg.substr(pos+1, end-pos-1);
                val.erase(std::remove_if(val.begin(), val.end(), ::isspace), val.end());
                return val == "true" || val == "1";
            }
        }
        return false;
    };
    info.battery = findInt("battery");
    info.chargeState = findBool("charge_state");
    return info;
}

BatteryInfo UDPClient::getBattery(int timeoutMs) {
    if (!sock) throw std::runtime_error("Socket not initialized");

    std::string msgStr = R"({"need_data":"battery,charge_state"})";

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

    int sent = sendto((SOCKET)sock, msgStr.c_str(), msgStr.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (sent == SOCKET_ERROR)
        throw std::runtime_error("sendto failed");

    DWORD timeout = timeoutMs;
    setsockopt((SOCKET)sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    char buffer[1024];
    sockaddr_in fromAddr{};
    int fromLen = sizeof(fromAddr);
    int recvLen = recvfrom((SOCKET)sock, buffer, sizeof(buffer)-1, 0, (sockaddr*)&fromAddr, &fromLen);
    if (recvLen == SOCKET_ERROR)
        throw std::runtime_error("No response from server, timeout!");

    buffer[recvLen] = '\0';
    std::string rcvStr(buffer);
    return parseBatteryResponse(rcvStr);
}

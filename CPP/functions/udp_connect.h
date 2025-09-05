#ifndef UDP_CONNECT_H
#define UDP_CONNECT_H

#pragma once
#include <string>
#include <thread>
#include <atomic>

struct BatteryInfo {
    int battery;        // 电量百分比
    bool chargeState;   // 是否充电
};

class UDPClient {
public:
    UDPClient(const std::string& ip, int port);
    ~UDPClient();

    // 请求 Battery 信息
    BatteryInfo getBattery(int timeoutMs = 2000);

private:
    std::string serverIP;
    int serverPort;
    void* sock;
};

std::thread start_udp_listener(int port, std::atomic<int>& heart_rate);

#endif // UDP_CONNECT_H
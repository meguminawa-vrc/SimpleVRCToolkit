#pragma once
#include <string>

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
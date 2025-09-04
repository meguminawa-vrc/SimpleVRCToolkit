#pragma once
#include <string>
#include <functional>

// 发送 OSC 消息
// ip: 目标 IP 地址
// port: 目标端口
// address: OSC 地址，例如 "/chatbox/input"
// msgText: 消息文本内容
// flag: 布尔参数，会在 OSC 消息类型标签中表示 T/F
void sendOSC(const std::string& ip, int port, const std::string& address, const std::string& msgText, bool flag);

// AFK 回调类型
using AFKCallback = std::function<void(bool)>;

// 启动 AFK 监听
// listenPort: 本地监听端口
// callback: 收到 AFK 参数变化时的回调函数
void startAFKListener(int listenPort, AFKCallback callback);
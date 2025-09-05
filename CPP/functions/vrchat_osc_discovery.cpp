#include "vrchat_osc_discovery.h"
#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>

namespace vrchat_osc {

    // 执行命令并获取输出
    std::string exec(const std::string& cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) throw std::runtime_error("popen() failed!");
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }

    // 调用外部可执行文件并解析输出
    std::pair<std::string, int> discover_ip(int timeout_seconds) {
        std::string command = "vrchat_osc_discovery.exe"; // 可执行文件路径
        std::string output = exec(command);

        // 简单解析输出
        if (output.find(":") != std::string::npos) {
            size_t pos = output.find(":");
            std::string ip = output.substr(0,pos);
            return {ip, 0}; // 0 表示成功
        } else {
            return {"127.0.0.1", 1}; // 默认 IP 和错误代码
        }
    }

} // namespace vrchat_osc

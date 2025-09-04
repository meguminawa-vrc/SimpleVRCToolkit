#include "battery.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp> // https://github.com/nlohmann/json

Battery::Battery(const std::string& file) : filename(file) {}

bool Battery::exists() const {
    std::ifstream f(filename);
    return f.good();
}

bool Battery::loadJson() {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;

        for (auto& [key, value] : j.items()) {
            int percent = std::stoi(key);
            double seconds = value["seconds"];
            std::string formatted = value["formatted"];
            data[percent] = BatteryEntry{seconds, formatted};

        }
    } catch (const std::exception& e) {
        std::cerr << "解析 JSON 出错: " << e.what() << std::endl;
        return false;
    }

    return true;
}

std::string Battery::getRemainingTime(int currentBattery) const {
    auto it = data.find(currentBattery);
    if (it != data.end()) {
        return it->second.formatted;
    } else {
        return "未知";
    }
}

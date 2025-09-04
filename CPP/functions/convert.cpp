#include "convert.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <nlohmann/json.hpp>

static std::string format_time(double seconds) {
    int s = static_cast<int>(seconds + 0.5); // 四舍五入
    if (s >= 3600) {
        int h = s / 3600;
        int m = (s % 3600) / 60;
        return std::to_string(h) + "小时" + std::to_string(m) + "分钟";
    } else if (s >= 60) {
        int m = s / 60;
        int sec = s % 60;
        return std::to_string(m) + "分钟" + std::to_string(sec) + "秒";
    } else {
        return std::to_string(s) + "秒";
    }
}

bool convertTempToJson(const std::string &txtFile, const std::string &jsonFile) {
    std::ifstream fin(txtFile);
    if (!fin.is_open()) {
        std::cerr << "无法打开 " << txtFile << std::endl;
        return false;
    }

    std::vector<std::pair<double, int>> records;
    std::string line;

    while (std::getline(fin, line)) {
        std::istringstream iss(line);
        double t;
        int battery;
        char percentChar;
        std::string secSymbol;

        if (!(iss >> t >> secSymbol >> battery >> percentChar)) continue;
        if (percentChar != '%') continue;

        records.emplace_back(t, battery);
    }

    if (records.empty()) {
        std::cerr << "文件为空或格式不正确" << std::endl;
        return false;
    }

    double t_end = records.back().first; // 最后时间
    std::map<int, double> firstSeen;

    for (auto &rec : records) {
        double t = rec.first;
        int battery = rec.second;
        if (firstSeen.find(battery) == firstSeen.end()) {
            firstSeen[battery] = t;
        }
    }

    nlohmann::json j;
    for (auto &kv : firstSeen) {
        int pct = kv.first;
        double t_battery = kv.second;
        double remain = t_end - t_battery;
        j[std::to_string(pct)] = {
            {"formatted", format_time(remain)},
            {"seconds", remain}
        };
    }

    std::ofstream fout(jsonFile);
    if (!fout.is_open()) {
        std::cerr << "无法写入 " << jsonFile << std::endl;
        return false;
    }
    fout << std::setw(4) << j << std::endl;

    return true;
}

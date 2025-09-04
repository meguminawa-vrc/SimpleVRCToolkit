#pragma once
#include <string>
#include <map>

struct BatteryEntry {
    double seconds;
    std::string formatted;
};

class Battery {
public:
    std::map<int, BatteryEntry> data;
    std::string filename;

    Battery(const std::string& file);

    bool exists() const;
    bool loadJson();
    std::string getRemainingTime(int currentBattery) const;
};

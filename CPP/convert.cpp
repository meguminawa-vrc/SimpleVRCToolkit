#include "functions/convert.h"
#include <iostream>

int main() {
    if (convertTempToJson("temp.txt", "battery_prediction.json")) {
        std::cout << "✅ 已生成 battery_prediction.json" << std::endl;
    } else {
        std::cerr << "❌ 转换失败" << std::endl;
    }
    return 0;
}
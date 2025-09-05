#include "functions/osc.h"
#include "functions/hardware.h"
#include "functions/function.h"
#include "functions/udp_connect.h"
#include "functions/battery.h"
#include "functions/INIReader.h"
#include "functions/vrchat_osc_discovery.h"
#include <string>
#include <iostream>
#include <thread>
#include <iomanip>
#include <windows.h>
#include <chrono>
#include <fstream>
#define SENDPORT 9000
#define LISTENPORT 9001
#define ADDRESS "/chatbox/input"

std::string IP = "127.0.0.1";
// 获取布尔输入
int getBoolInput(const std::string& prompt) {
    int input;
    while (true) {
        std::cout << prompt << " (0/1): ";
        if (std::cin >> input && (input == 0 || input == 1)) {
            return input;
        } else {
            std::cout << "输入无效，请输入 0 或 1\n";
            std::cin.clear();
            std::cin.ignore(1000, '\n');
        }
    }
}

// 打印数据行，并保持行首覆盖
void printDataLine(const std::string& line) {
    std::cout << "\r" << std::setw(120) << std::left << line << std::flush;
}


void createDefaultINI(const std::string &filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法创建文件: " << filename << std::endl;
        return;
    }

    file << "[General]\n";
    file << "android_state = 2 \n#2表示在程序启动时询问,0表示不询问默认选择0,1表示不询问默认选择1\n";
    file << "battery_log_state = 2 \n#2表示在程序启动时询问,0表示不询问默认选择0,1表示不询问默认选择1\n";
    file << "refresh_rate = 2 \n#刷新率\n";
    file << "charge_string = (充电中) \n#充电时显示的文字\n";
    file << "default_string = 活的很好  \n#默认状态下状态栏显示的文字\n";
    file << "lowrate_string = 要被卡死了  \n#游戏卡的时候状态栏显示的文字\n";
    file << "afk_string = 亖了  \n#挂机的时候状态栏显示的文字\n";
    file << "low_battery_string = 要没电哩  \n#电量低的时候状态栏显示的文字\n";
    file.close();
}


int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
     std::string configFile = "config.ini";

        // 尝试读取
        INIReader reader(configFile);
        if (reader.ParseError() < 0) {
            std::cout << "INI 文件不存在，创建默认配置...\n";
            createDefaultINI(configFile);
            reader = INIReader(configFile); // 再次加载
        }

    
    int android_state = reader.GetInteger("General", "android_state", 2);
    int battery_log_state = reader.GetInteger("General", "battery_log_state", 2);
    float refresh_rate = reader.GetReal("General", "refresh_rate", 2.0);
    std::string charge_string = reader.Get("General", "charge_string", "(充电中)");
    std::string default_string = reader.Get("General", "default_string", "活的很好");
    std::string lowrate_string = reader.Get("General", "lowrate_string", "要被卡死了");
    std::string afk_string = reader.Get("General", "afk_string", "亖了");
    std::string low_battery_string = reader.Get("General", "low_battery_string", "要没电哩");

    std::ofstream file_t("temp.txt", std::ios::out);
    
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();


    bool is_afk = false;


    std::cout << "可以在config.ini更改设置" << std::endl;


    auto [ip, status] = vrchat_osc::discover_ip();
    if (status == 0) {
        std::cout << "在 " << ip << " 上发现 OSC 服务器\n";
        IP = ip;
    } else {
        std::cout << "OSC 服务器未发现，使用默认 127.0.0.1\n";
    }


    bool android_flag = false;
    bool battery_log_flag = false;


    switch(android_state){
        case 2:
            android_flag = getBoolInput("是否要检测头显端参数(只有当绘制过曲线存在battery_prediction.json文件才会显示还能使用多长时间)");
            break;
        case 1:
            android_flag = true;
            break;
        case 0:
            android_flag = false;
            break;
    }
    switch(battery_log_state){
        case 2:
            battery_log_state = getBoolInput("是否要画电池曲线(如果要绘制曲线请先将电充满记录至关机,再运行convert.exe)");
            break;
        case 1:
            battery_log_state = true;
            break;
        case 0:
            battery_log_state = false;
            break;
    }


    int battery_pct;
    std::string android_ip;
    UDPClient* client = nullptr;


    if(android_flag){
        std::cout << "请输入头显端ip地址: ";
        std::cin >> android_ip;
        client = new UDPClient(android_ip, 9999);
    }

    startAFKListener(LISTENPORT, [&](bool afk) {
        is_afk = afk;
    });

    while(true){
        GPUInfo gpu = getGPUInfo(0);
        CPUInfo cpu = getCPUAndMemInfo();

        float gpu_usage = gpu.gpuUtil;
        float vram_usage = gpu.memUsed / gpu.memTotal;
        float mem_usage = cpu.memPercent;

        std::string osc_msg = "显卡: " + toString(gpu_usage) + "% | 显存: " 
            + toString(static_cast<int>(vram_usage*100)) + "% | 内存: " + toString(mem_usage) + "%";

        if(android_flag && client){
            try {
                BatteryInfo info = client->getBattery(2000);
                battery_pct = info.battery;
                osc_msg += " | 头显电量剩余: " + std::to_string(info.battery) + "%";

                if(info.chargeState){
                    osc_msg += " ";
                    osc_msg += charge_string;
                }
                else {
                    try {
                        Battery battery("battery_prediction.json");

                        if (!battery.exists()) {
                            std::cerr << "[ERROR] 文件 battery_prediction.json 不存在！" << std::endl;
                        }
                        else if (!battery.loadJson()) {
                            std::cerr << "[ERROR] JSON 解析失败，请检查文件内容。" << std::endl;
                        }
                        else {
                            std::string remaining = battery.getRemainingTime(info.battery);
                            osc_msg += " | 还能使用 " + remaining;
                        }
                    } catch(const std::exception& e) {
                        std::cerr << "[EXCEPTION] 解析 battery_prediction.json 出错: " << e.what() << std::endl;
                        osc_msg += " | 电池信息异常";
                    }
                }
                if (battery_log_flag) {
                    // 计算运行时间（秒）
                    auto now = std::chrono::steady_clock::now();
                    float time_sec = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() / 1000.0f;
                    // 追加写入 temp.txt
                    std::ofstream file("temp.txt", std::ios::app);
                    if (file.is_open()) {
                        file << toString(time_sec) << "s " 
                            << toString(info.battery) << "%\n";
                    } else {
                        std::cerr << "无法打开 temp.txt 进行写入" << std::endl;
                    }
                }
            } 
            catch(const std::exception& e) {
                std::cerr << "[EXCEPTION] 获取设备电池信息失败: " << e.what() << std::endl;
                osc_msg += " | 电池信息异常";
            }
    
        }
        std::string condition = default_string;
        if(gpu.gpuUtil >= 98){
            condition = lowrate_string;
        }
        if(is_afk){
            condition = afk_string;
        }
        if (android_flag && client && battery_pct <= 10){
            condition += ",";
            condition += low_battery_string;
        }
        osc_msg += " 当前状态: ";
        osc_msg += condition;


        
        // 发送 OSC
        sendOSC(IP, SENDPORT, ADDRESS, osc_msg, true);

        // 输出到控制台
        printDataLine(osc_msg);

        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(1000/refresh_rate)));
    }

    if(client) delete client;
    #ifdef OS_WINDOWS
    WSACleanup();
    #endif
    return 0;
}

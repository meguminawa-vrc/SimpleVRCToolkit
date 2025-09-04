#include "hardware.h"
#include <windows.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <array>
#include <memory>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <cstdlib>
#include<pdh.h>
#pragma comment(lib, "pdh.lib") 

// -------- GPU --------
static std::string execCommand(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
    if (!pipe) throw std::runtime_error("Failed to run command");

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

GPUInfo getGPUInfo(int index) {
    std::string cmd = "nvidia-smi --query-gpu=name,memory.total,memory.used,memory.free,utilization.gpu,utilization.memory --format=csv,noheader,nounits";
    std::string output = execCommand(cmd);

    std::istringstream ss(output);
    std::string line;
    std::vector<GPUInfo> gpus;

    while (std::getline(ss, line)) {
        std::istringstream ls(line);
        std::string token;
        GPUInfo gpu;
        int i = 0;
        while (std::getline(ls, token, ',')) {
            token.erase(0, token.find_first_not_of(" \t"));
            token.erase(token.find_last_not_of(" \t") + 1);
            switch (i) {
                case 0: gpu.name = token; break;
                case 1: gpu.memTotal = std::stof(token); break;
                case 2: gpu.memUsed  = std::stof(token); break;
                case 3: gpu.memFree  = std::stof(token); break;
                case 4: gpu.gpuUtil  = std::stof(token); break;
                case 5: gpu.memUtil  = std::stof(token); break;
            }
            i++;
        }
        gpus.push_back(gpu);
    }

    if (index >= 0 && index < gpus.size()) return gpus[index];
    throw std::runtime_error("GPU index out of range");
}

// -------- CPU & Memory --------

// 将 FILETIME 转为 ULONGLONG
static ULONGLONG fileTimeToULL(const FILETIME& ft) {
    return (((ULONGLONG)ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}

// 获取 CPU 使用率（PDH）和内存占用
// 全局静态变量保持 PDH 查询状态
static PDH_HQUERY g_query = nullptr;
static PDH_HCOUNTER g_counter;
static bool g_initialized = false;

// 初始化 PDH 查询（只需调用一次）
void initCPUQuery() {
    if (!g_initialized) {
        PdhOpenQuery(NULL, 0, &g_query);
        PdhAddCounterW(g_query, L"\\Processor(_Total)\\% Processor Time", 0, &g_counter);
        PdhCollectQueryData(g_query); // 第一次采样
        g_initialized = true;
    }
}

// 获取 CPU 和内存占用
CPUInfo getCPUAndMemInfo() {
    if (!g_initialized) {
        initCPUQuery();
        // 等待 200ms 确保第一次采样有效
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    PdhCollectQueryData(g_query);
    PDH_FMT_COUNTERVALUE value;
    PdhGetFormattedCounterValue(g_counter, PDH_FMT_DOUBLE, NULL, &value);
    double cpuPercent = value.doubleValue;

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    float memPercent = static_cast<float>(memInfo.dwMemoryLoad);

    return CPUInfo{ cpuPercent, memPercent };
}
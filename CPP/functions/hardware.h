#pragma once
#include <string>

struct GPUInfo {
    std::string name;
    float memTotal;
    float memUsed;
    float memFree;
    float gpuUtil;
    float memUtil;
};

struct CPUInfo {
    double cpuPercent;
    float memPercent;
};

GPUInfo getGPUInfo(int index = 0);
CPUInfo getCPUAndMemInfo();
void initCPUQuery();
#include "function.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <regex>
#include <windows.h>



std::string formatOSCMessage(
    const std::string& templateStr, 
    const std::unordered_map<std::string, std::string>& params
)
{
    std::string result;
    std::string keyBuffer;
    bool inBraces = false;

    for (size_t i = 0; i < templateStr.size(); ++i) {
        char c = templateStr[i];

        if (c == '{') {
            if (inBraces) {
                // 遇到嵌套 {，输出原文本
                result += '{' + keyBuffer;
                keyBuffer.clear();
            }
            inBraces = true;
        }
        else if (c == '}' && inBraces) {
            inBraces = false;
            auto it = params.find(keyBuffer);
            if (it != params.end()) {
                result += it->second;
            } else {
                std::cerr << "Warning: 未找到参数 {" << keyBuffer << "}" << std::endl;
                result += "{" + keyBuffer + "}"; // 保留原文本
            }
            keyBuffer.clear();
        }
        else if (inBraces) {
            // 只收集合法标识符字符：字母、数字、下划线
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') {
                keyBuffer += c;
            }
            // 非法字符直接忽略，不会造成未闭合
        }
        else {
            result += c; // 普通字符直接加入
        }
    }

    // 如果仍在大括号内，说明确实缺少闭合
    if (inBraces && !keyBuffer.empty()) {
        std::cerr << "Warning: 遇到未闭合的大括号 {" << keyBuffer << "}" << std::endl;
        result += "{" + keyBuffer; 
    }

    return result;
}

void run_hidden(const wchar_t* exePath) {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // 隐藏窗口

    if (!CreateProcessW(
            exePath,       // 应用程序路径 (宽字符)
            NULL,          // 命令行参数
            NULL, NULL,    // 安全属性
            FALSE,         // 是否继承句柄
            0,             // 创建标志
            NULL,          // 环境变量
            NULL,          // 当前目录
            &si,
            &pi
        )) {
        MessageBoxW(NULL, L"启动失败！", L"Error", MB_ICONERROR);
    } else {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}


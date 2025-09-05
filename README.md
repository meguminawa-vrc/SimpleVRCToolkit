# SimpleVRCToolkit

一个由@meguminawa为 VRChat 用户开发的简单工具包，主要功能是 **获取 VR 头显的电池电量信息与电脑上部分配件信息甚至手表上的心率信息** 并通过 VRCOSC发送。  
最初是 Python 脚本版本，后来增加了 C++ 实现，方便没有 Python 环境的用户使用。  
安卓端提供了一个 UDP 服务器 App，可以直接在头显上运行并上报电池状态。

---

示例:
<center>
  <img width="569" height="658" alt="VRChat_2025-09-06_04-08-25" src="https://github.com/user-attachments/assets/25f5f079-fbe4-4517-b3c0-427edf8d98ce">
</center>


---
## 📂 项目结构

    SimpleVRCToolkit/
    ├── CPP/ # C++ 源代码
    │   ├── APIs/  # 使用的其他语言api源代码
    │   │   ├── heart_beat_udp_server.py # hyperate心率广播源代码转发UDP服务器源代码
    │   │   └── vrchat_osc_discovery.py  # mDNS监测程序源代码
    │   ├── functions/ # 使用到的函数库
    │   │   └──...
    │   ├── convert.cpp # 将temp.txt转化为battery_prediction.json程序的源代码
    │   ├── gcc.txt #使用到的gcc构建命令
    │   └── main.cpp #主函数源代码
    ├── Python/          # Python 源代码
    │   ├── heartrate.py # hyperate心率广播源代码转发UDP服务器源代码
    │   └── main.py 主函数源代码
    ├── Android/         # 安卓端源码 (MainActivity.java / AndroidManifest.xml / UdpServerService.java)
    ├── output/
    │   ├── exe/         # C++ 编译结果
    │   │   ├── main.exe # 主程序
    │   │   ├── convert.exe #将temp.txt转化为battery_prediction.json的程序
    │   │   ├── vrchat_osc_descovery.exe # mDNS监测程序
    │   │   ├── heart_beat_udp_server.exe # hyperate心率广播源代码转发UDP服务器
    │   │   └── msedgedriver.exe # selenium的edge浏览器驱动
    │   └── android/     # 安卓 APK
    │       └── SimpleVRCToolkitUDPServer.apk
    └── README.md

---

## 🚀 使用方法

### 一. 使用 Python 版本（推荐，有 Python 环境的用户）
  1. 进入 `Python/` 目录，直接运行 Python 脚本：
        ```bash
        python main.py

  2.参数都可以在源码中修改。

  3.如果要连接头显的 UDP 服务器，请修改代码里的 IP 地址。

### 二.使用 C++ 编译版本（无 Python 环境的用户）
  1.下载 outputs/exe/ 下的五个文件：
    main.exe
    convert.exe
    vrchat_osc_descovery.exe
    heart_beat_udp_server.exe
    msedgedriver.exe

  2.将它们放在一个单独的文件夹中运行（第一次运行会生成必要文件）。

  3.运行 main.exe 后会进入命令行交互：
    第一次运行后 会生成 config.ini，可在其中修改配置（例如默认选项、显示的文字等）。
    可以避免每次启动都重复选择。

### 三. 获取头显电池信息
  1.在头显中安装并运行 outputs/android/SimpleVRCToolkitUDPServer.apk。

  2.启动 main.exe时：
    第一个选项选择 1
    输入头显上显示的 IP 地址

  3.即可获取电池电量数据。

### 四.电池预测（剩余关机时间）
  1.校准过程：
    先将头显充满电。
    在 main.exe 第二个选项选择 1。
    它会记录放电过程中的电量曲线，数据保存在 temp.txt。
    等到头显自动关机后停止记录。

  2.生成预测模型：
    当 temp.txt 中保存了 100% → 1% 的完整数据时，运行：
      convert.exe
    会生成 battery_prediction.json。

  3.之后再打开电池信息时，如果不是在充电状态，它会显示预计剩余使用时间。

### 五.心率检测
  1.确保你的手表和手机都下载了hyperate并且打开心率广播

  2.打开config.ini将 heart_rate_flag 设为1 并且在 heart_rate_session_id 填入心率广播的 sessionid,即它给的网站 https://app.hyperate.io/* *代表的部分

  3.重启软件
## ⚙️ 配置文件说明
运行程序后会生成一个 `config.ini` 文件，用于保存用户的默认设置。  
你可以根据需要修改其中的值，避免每次运行都重复选择。
    
### 示例配置 (`config.ini`)
    ```ini
    [General]
        
    # Android 连接模式选项
    # 0 = 不询问，默认选择 "关闭 Android 端 UDP 连接"
    # 1 = 不询问，默认选择 "开启 Android 端 UDP 连接"
    # 2 = 每次程序启动时都询问用户
    android_state = 2 
        
    # 电池记录功能开关
    # 0 = 不询问，默认关闭电池曲线记录
    # 1 = 不询问，默认开启电池曲线记录
    # 2 = 每次程序启动时都询问用户是否开启
    battery_log_state = 2 
        
    # 刷新率（次/秒）
    # 控制OSC更新的间隔，数字越小刷新越快，速度过快会触发VRC的短暂(30s)禁言,实测0.5不会触发禁言。
    refresh_rate = 2
        
    # 自定义状态栏显示的文字
    charge_string = (充电中)        # 设备正在充电时显示的文字
    default_string = 活的很好       # 默认状态下显示的文字
    lowrate_string = 要被卡死了喵   # 游戏掉帧或卡顿时显示的文字
    afk_string = 亖了               # 用户挂机（AFK）时显示的文字
    low_battery_string = 要没电哩   # 电池电量过低时显示的文字

    
    show_GPU_name = 1               #是否要显示GPU名称
    android_ip = null               #头显端再局域网内的ip,若没填则再程序运行时询问,若填了则运行时自动使用这个地址
    heart_rate_flag = 0             #是否要启用心率检测,0为不启用1为启用,若启用则要使用hyperrate开启心率广播并且填入session_id
    heart_rate_port = 10001         #python转发心率广播的UDP端口
    heart_rate_session_id = 0       #hyperrate开启心率广播后显示的sessionid
    




## 📌 注意事项
  第一次运行 main.exe 建议放到一个空文件夹里，避免生成的文件混乱。
  安卓 APK 需要手动安装到头显上（开启开发者模式/允许未知来源应用）。
  如果要使用心率监测功能请在手表和手机上打开HypeRate软件并且打开心率广播 。
  如果使用 Python 版本，请确保 Python 环境已安装（推荐 Python 3.9+）。

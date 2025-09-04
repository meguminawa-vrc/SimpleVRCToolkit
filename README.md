# SimpleVRCToolkit

一个由@meguminawa为 VRChat 用户开发的简单工具包，主要功能是 **获取 VR 头显的电池电量信息与电脑上部分配件信息** 并通过 VRCOSC发送。  
最初是 Python 脚本版本，后来增加了 C++ 实现，方便没有 Python 环境的用户使用。  
安卓端提供了一个 UDP 服务器 App，可以直接在头显上运行并上报电池状态。

---

## 📂 项目结构

    SimpleVRCToolkit/
    ├── CPP/             # C++ 源代码
    ├── Python/          # Python 源代码
    ├── Android/         # 安卓端源码 (MainActivity.java / AndroidManifest.xml / UdpServerService.java)
    ├── outputs/
    │   ├── exe/         # C++ 编译结果
    │   │   ├── SimpleVRCToolkit.exe
    │   │   └── convert.exe
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
  1.下载 outputs/exe/ 下的两个文件：
    SimpleVRCToolkit.exe
    convert.exe

  2.将它们放在一个单独的文件夹中运行（第一次运行会生成必要文件）。

  3.运行 SimpleVRCToolkit.exe 后会进入命令行交互：
    第一次运行后 会生成 config.ini，可在其中修改配置（例如默认选项、显示的文字等）。
    可以避免每次启动都重复选择。

### 三. 获取头显电池信息
  1.在头显中安装并运行 outputs/android/SimpleVRCToolkitUDPServer.apk。

  2.启动 SimpleVRCToolkit.exe 或 Python 脚本时：
    第一个选项选择 1
    输入头显上显示的 IP 地址

  3.即可获取电池电量数据。

### 四.电池预测（剩余关机时间）
  1.校准过程：
    先将头显充满电。
    在 SimpleVRCToolkit.exe 第二个选项选择 1。
    它会记录放电过程中的电量曲线，数据保存在 temp.txt。
    等到头显自动关机后停止记录。

  2.生成预测模型：
    当 temp.txt 中保存了 100% → 1% 的完整数据时，运行：
      convert.exe
    会生成 battery_prediction.json。

  3.之后再打开电池信息时，如果不是在充电状态，它会显示预计剩余使用时间。


## ⚙️ 配置文件说明
  config.ini
  第一次运行后自动生成。
  可以修改默认选项、显示文本等。
  如果不想每次选择菜单，可以直接在里面配置默认值。


## 📌 注意事项
  第一次运行 SimpleVRCToolkit.exe 建议放到一个空文件夹里，避免生成的文件混乱。
  安卓 APK 需要手动安装到头显上（开启开发者模式/允许未知来源应用）。
  如果使用 Python 版本，请确保 Python 环境已安装（推荐 Python 3.9+）。

#!D:\Codes\Anaconda3\envs\VRChat\python.exe
from pythonosc import udp_client,dispatcher, osc_server
from zeroconf import Zeroconf, ServiceBrowser, ServiceListener
import argparse,psutil,time,socket,json,threading,subprocess
import heartrate
#连接头显UDP服务器
UDP_IP = "0.0.0.0"  # 头显局域网 IP
UDP_PORT = 9999            # UDP 服务器监听端口
HEART_RATE_FLAG = True #是否开启心率检测
HEART_PORT = 10001
with open('battery.txt','w') as b:
        b.write('')
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(5)  # 超时时间 5 秒



start_time = time.time()  #脚本启动时的时间戳

is_afk= False  #afk状态,global
heart_rate = 0 #心率,global

def start_udp_listener(listen_port=10001):
    """
    启动UDP监听线程，当收到消息时更新全局变量 `heart_rate`
    listen_port: 本地监听端口
    """
    global heart_rate

    def listener_thread():
        global heart_rate
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind(("0.0.0.0", listen_port))
        print(f"UDP listener started on port {listen_port}")

        while True:
            data, addr = sock.recvfrom(1024)  # 接收1024字节以内的消息
            try:
                # 尝试解析为字符串
                heart_rate = data.decode("utf-8")
            except Exception:
                # 若解析失败，直接保存原始字节
                heart_ratee = data
            print(f"Received from {addr}: {heart_rate}")

    thread = threading.Thread(target=listener_thread, daemon=True)
    thread.start()
    return thread


def afk_handler(addr, value):
    global is_afk
    is_afk = bool(value)
    print(f"接收到 AFK 状态更新: {is_afk}")

def start_afk_listener(port=9001):
    global server
    disp = dispatcher.Dispatcher()
    disp.map("/avatar/parameters/AFK", afk_handler)

    server = osc_server.ThreadingOSCUDPServer(("0.0.0.0", port), disp)
    print(f"OSC 监听 AFK 参数中（端口 {port}）...")
    server.serve_forever()
def get_gpu_info():

    result = subprocess.check_output(
        [
            "nvidia-smi",
        "--query-gpu=name,memory.total,memory.used,memory.free,utilization.gpu,utilization.memory",
                "--format=csv,noheader,nounits"
        ],
        encoding="utf-8"
    )
        
    gpus = result.strip().split("\n")
    gpu_info = []
    for gpu in gpus:
        name, mem_total, mem_used, mem_free, util_gpu, util_mem = gpu.split(", ")
        gpu_info.append({
            "显卡型号": name,
            "显存总量 (MB)": int(mem_total),
            "已用显存 (MB)": int(mem_used),
            "空闲显存 (MB)": int(mem_free),
            "GPU利用率 (%)": int(util_gpu),
            "显存利用率 (%)": int(util_mem)
        })
    return gpu_info[0]
def get_cpu_and_mem_info():
    cpu_per = 0
    for i in psutil.cpu_percent(interval=1,percpu=True):
        cpu_per += i
    cpu_per = int(cpu_per*10)/10
    vm = psutil.virtual_memory()
    mem_per = str(vm.percent)+"%"
    info = {}
    info['cpu_per'] = cpu_per
    info['mem_per'] = mem_per
    return info
def get_osc_info():
    battery,charge_state = get_battery()
    if battery:
        log_battery(battery)
    gpu_info = get_gpu_info()
    gpu_used = gpu_info['GPU利用率 (%)']
    mem_used= gpu_info["已用显存 (MB)"]
    mem_total = gpu_info["显存总量 (MB)"]
    gpu_name = gpu_info['显卡型号']
    gmem_util = gpu_info["显存利用率 (%)"]
    info = get_cpu_and_mem_info()
    cpu_per = info['cpu_per']
    mem_per = info['mem_per']
    print(f'显卡型号:{gpu_name}')
    print(f"GPU利用率: {gpu_used}%")
    print(f"显存占用: {mem_used} MB / {mem_total} MB")
    print(f'显存占用率:{int(mem_used*100/mem_total)}%')
    print(f"CPU占用: {cpu_per}%")
    print(f"内存占用: {mem_per}%")
    vram = int(mem_used*100/mem_total)
    state = '活得很好'
    if gpu_used >= 98:
        state = '要被卡死了'
    if is_afk:
        state = '亖了'
    if int(battery) <= 10:
        state += ',要没电哩！'
    osc_msg = f'CPU:{cpu_per}% GPU:{gpu_used}%\nRAM:{mem_per} VRAM:{vram}%\n当前状态: {state} \n头显剩余电量:{battery}%'
    fake_osc_msg = f'CPU:{int((cpu_per/5)*10)/10}% GPU:{gpu_used/5}%\nRAM:{mem_per} VRAM:{vram/5}%\n当前状态: {state} \n头显剩余电:{battery}%'
    if charge_state:
        osc_msg += ' 正在充电中'
    else:
        osc_msg += f" 还能使用 {predict_time(battery)['formatted']} "

    if HEART_RATE_FLAG:
        osc_msg += f" |当前心率 ❤{heart_rate}"
    return osc_msg,fake_osc_msg


class _OSCListener(ServiceListener):
    def __init__(self):
        self.service_info = None

    def add_service(self, zeroconf, type_, name):
        info = zeroconf.get_service_info(type_, name)
        if info:
            self.service_info = info

def get_vrchat_osc_ip(timeout=5.0):
    """
    Discover VRChat OSC service via mDNS and return (ip, port).
    Returns (None, None) if not found within timeout.
    """
    zeroconf = Zeroconf()
    listener = _OSCListener()
    browser = ServiceBrowser(zeroconf, "_osc._udp.local.", listener)

    end_time = time.time() + timeout
    while time.time() < end_time:
        info = listener.service_info
        if info:
            # 从 addresses 获取 IPv4 字节，然后转换为字符串
            addrs = info.addresses
            if addrs:
                ip = socket.inet_ntoa(addrs[0])  # 使用第一个 IPv4 地址
                port = info.port
                zeroconf.close()
                return ip, port

            # 或者直接使用解析后的字符串
            parsed = info.parsed_addresses()
            if parsed:
                ip = parsed[0]
                port = info.port
                zeroconf.close()
                return ip, port
        time.sleep(0.1)

    zeroconf.close()
    return None, None



parser = argparse.ArgumentParser()
ip, port = get_vrchat_osc_ip(timeout=5.0)
if ip:
    print(f"VRChat OSC 服务发现于 {ip}:{port}")
else:
    ip = "127.0.0.1"
    print("在网络中未能发现 VRChat OSC 服务，请确保 VRChat 已启用 OSC 功能。")
parser.add_argument("--ip", default=ip)
parser.add_argument("--port", type=int, default=9000)
args = parser.parse_args()

client = udp_client.SimpleUDPClient(args.ip, args.port)
def get_battery():
    try:
        # 发送消息到 UDP 服务器
        msg = {
            "need_data": "battery,charge_state"
        }
        message = json.dumps(msg)
        sock.sendto(message.encode(), (UDP_IP, UDP_PORT))  # encode() 转为 bytes
        print(f"Sent: {message}")

        # 等待服务器响应
        data, addr = sock.recvfrom(1024)
        print(f"Received from {addr}: {data.decode()}")
        rcv = json.loads(data.decode())
        return rcv['battery'],rcv['charge_state']

    except socket.timeout:
        print("No response from server, timeout!")
        return 100,1
def log_battery(battery:int):
    now_time = time.time()
    use_time = now_time - start_time
    with open('battery.txt','a') as b:
        b.write(f'{use_time}s  {battery}%\n')

def predict_time(remain_pct: float, json_file="battery_prediction.json"):
    """传入电量百分比，返回预计剩余时间"""
    with open(json_file, "r", encoding="utf-8") as f:
        data = json.load(f)

    # 提取并排序
    pcts = sorted(int(k) for k in data.keys())
    times = [data[str(p)]["seconds"] for p in pcts]

    if remain_pct in pcts:  # 恰好有记录
        t = data[str(int(remain_pct))]["seconds"]
    elif remain_pct > max(pcts):  # 高于最大记录
        t = times[0]
    elif remain_pct < min(pcts):  # 低于最小记录
        t = times[-1]
    else:
        # 线性插值
        for i in range(len(pcts) - 1):
            if pcts[i] >= remain_pct >= pcts[i + 1]:
                x1, y1 = pcts[i], times[i]
                x2, y2 = pcts[i + 1], times[i + 1]
                # y = y1 + (y2-y1)*(x-x1)/(x2-x1)
                t = y1 + (y2 - y1) * (remain_pct - x1) / (x2 - x1)
                break

    return {"seconds": round(t, 1), "formatted": format_time(t)}
def format_time(seconds: float) -> str:
    """把秒数格式化成人类可读的时间"""
    seconds = int(round(seconds))
    if seconds >= 3600:
        h = seconds // 3600
        m = (seconds % 3600) // 60
        return f"{h}小时{m}分钟"
    elif seconds >= 60:
        m = seconds // 60
        s = seconds % 60
        return f"{m}分钟{s}秒"
    else:
        return f"{seconds}秒"




def main():
    # 启动监听线程
    listener_thread = threading.Thread(target=start_afk_listener, daemon=True)
    listener_thread.start()
    if HEART_RATE_FLAG:
        listener_thread_heart = threading.Thread(target=start_udp_listener, daemon=True,args=[HEART_PORT,])
        listener_thread_heart.start()
        heartrate.start_heart_monitor(HEART_PORT)
    while True:
        print(f'afk:{is_afk}\n')
        osc_msg,fake_osc_msg = get_osc_info()
        #client.send_message("/chatbox/input",[fake_osc_msg, True])
        client.send_message("/chatbox/input",[osc_msg, True])
        time.sleep(0.5)


main()
sock.close()
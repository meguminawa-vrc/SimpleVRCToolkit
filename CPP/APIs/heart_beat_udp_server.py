#!D:\Codes\Anaconda3\envs\spider\python.exe
import socket,os
import threading
import time
from selenium import webdriver
from selenium.webdriver.edge.options import Options
from selenium.webdriver.edge.service import Service
import tempfile
import configparser


config = configparser.ConfigParser()
config.read('config.ini', encoding='utf-8')


heart_rate_flag = int(config['General'].get('heart_rate_flag'))
udp_port = int(config['General'].get('heart_rate_port'))
heart_rate_session_id = int(config['General'].get('heart_rate_session_id'))


HEART_BEAT_URL= f'https://app.hyperate.io/{heart_rate_session_id}'


def start_heart_monitor(udp_port=10001):
    """
    启动 headless Edge 浏览器抓取 Hyperate 心率数据，并通过本地 UDP 发送。
    
    udp_port: 本地 UDP 端口
    """
    # --- 设置 UDP ---
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_address = ('127.0.0.1', udp_port)

    # --- 设置 headless Edge ---
    edge_options = Options()
    edge_options.add_argument('--headless')
    edge_options.add_argument('--disable-gpu')
    edge_options.add_argument('--ignore-certificate-errors')
    edge_options.add_argument('--disable-web-security')
    # 每次独立临时数据目录，避免冲突
    user_data_dir = tempfile.mkdtemp()
    edge_options.add_argument(f'--user-data-dir={user_data_dir}')
    cwd = os.getcwd()
    service = Service(f'{cwd}/msedgedriver.exe')
    driver = webdriver.Edge(service=service, options=edge_options)

    # 打开页面
    driver.get(HEART_BEAT_URL)

    # --- 心率监控线程 ---
    def monitor_heart_rate():
        last_value = None
        while True:
            try:
                hr_element = driver.find_element("css selector", 'p.heartrate')
                hr_text = hr_element.text.strip()
                if hr_text and hr_text != last_value:
                    last_value = hr_text
                    # 发送到本地 UDP
                    sock.sendto(hr_text.encode('utf-8'), udp_address)
                    print(f"Heart rate: {hr_text} -> sent via UDP:{udp_port}", flush=True)
            except Exception as e:
                # 忽略元素还没加载的异常
                pass
            time.sleep(5)  # 每5秒抓一次

    thread = threading.Thread(target=monitor_heart_rate, daemon=True)
    thread.start()

    return sock, thread

# --- 调用 ---


if __name__ == "__main__":
    sock, thread = start_heart_monitor(udp_port)
    print("Heart monitor started. Press Ctrl+C to exit.")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Exiting...")

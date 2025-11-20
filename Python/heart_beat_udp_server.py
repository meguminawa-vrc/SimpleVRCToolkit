#!D:\Codes\Anaconda3\envs\VRChat\python.exe
import socket,os, shutil
import threading
import time
from selenium import webdriver
from selenium.webdriver.edge.options import Options
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.edge.service import Service
import tempfile
import configparser
import sys
import requests
import subprocess
import glob


config = configparser.ConfigParser()
config.read('config.ini', encoding='utf-8')


heart_rate_flag = int(config['General'].get('heart_rate_flag'))
udp_port = int(config['General'].get('heart_rate_port'))
heart_rate_session_id = config['General'].get('heart_rate_session_id')


HEART_BEAT_URL= f'https://app.hyperate.io/{heart_rate_session_id}'


def start_heart_monitor(udp_port=10001):
    """
    启动 headless Edge 访问新页面，并把 #heartRate 的值通过本地 UDP 发送
    """
    # --- UDP ---
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_address = ("127.0.0.1", udp_port)

    # --- Edge headless ---
    edge_options = Options()
    # 新版 headless 更稳
    edge_options.add_argument("--headless=new")
    edge_options.add_argument("--disable-gpu")
    edge_options.add_argument("--ignore-certificate-errors")
    # 可选：不一定需要，减少副作用
    # edge_options.add_argument("--disable-web-security")

    # 单次独立 profile，避免缓存/登录状态干扰
    user_data_dir = tempfile.mkdtemp(prefix="hyperate_")
    edge_options.add_argument(f"--user-data-dir={user_data_dir}")
    driver_path = ensure_latest_edge_driver()
    service = Service(executable_path=driver_path)
    driver = webdriver.Edge(service=service, options=edge_options)

    # --- 打开页面（先拿重定向后的真实 URL）---
    NEW_URL = return_new_url(HEART_BEAT_URL) or HEART_BEAT_URL
    driver.get(NEW_URL)

    # 等 #heartRate 节点出现
    wait = WebDriverWait(driver, 20)
    hr_elem = wait.until(EC.presence_of_element_located((By.CSS_SELECTOR, "#heartRate")))

    # 也可以给页面注入一个 MutationObserver，把最新值放到 window._latestHR
    driver.execute_script("""
        window._latestHR = document.querySelector('#heartRate')?.innerText?.trim() || '';
        const target = document.querySelector('#heartRate');
        if (target) {
            const obs = new MutationObserver(() => {
                window._latestHR = target.innerText.trim();
            });
            obs.observe(target, { characterData: true, childList: true, subtree: true });
        }
    """)

    def get_hr_text():
        # 读注入的变量，避免频繁 DOM 查找
        try:
            return driver.execute_script("return window._latestHR || (document.querySelector('#heartRate')?.innerText?.trim() || '');")
        except Exception:
            return ""

    # --- 监控线程 ---
    def monitor_heart_rate():
        last_value = None
        while True:
            try:
                hr_text = get_hr_text()
                # 只发纯数字/非空，避免初始 0 或空白
                if hr_text and hr_text.isdigit() and hr_text != last_value:
                    last_value = hr_text
                    sock.sendto(hr_text.encode("utf-8"), udp_address)
                    sys.stdout.write(f"\rHeart rate: {hr_text} -> UDP:{udp_port}   ")
                    sys.stdout.flush()
            except Exception:
                pass
            time.sleep(1.5)  # 频率稍高一点更实时

    thread = threading.Thread(target=monitor_heart_rate, daemon=True)
    thread.start()

    # 可选：提供一个清理函数（关闭后删临时目录）
    def _cleanup():
        try:
            driver.quit()
        finally:
            try:
                shutil.rmtree(user_data_dir, ignore_errors=True)
            except Exception:
                pass

    # 你可以把 _cleanup 返回/保存以便主程序退出时调用
    return sock, thread

# --- 调用 ---

def return_new_url(url):
    # 关键：关闭自动重定向
    response = requests.get(url, allow_redirects=False)
    
    # 如果是重定向响应 (301 / 302 / 307 / 308)
    if response.is_redirect or response.is_permanent_redirect:
        return response.headers.get('Location')
    else:
        return None
    


def ensure_latest_edge_driver():
    """
    1) 在当前目录执行 selenium-manager.exe --browser edge（如果存在）
    2) 在默认缓存目录下自动找到最新的 EdgeDriver
    3) 返回最新 msedgedriver.exe 的绝对路径
    """

    # --- Step 1: 尝试执行 selenium-manager.exe 下载 driver ---
    manager_path = os.path.join(os.getcwd(), "selenium-manager.exe")

    if os.path.isfile(manager_path):
        print("[INFO] Running selenium-manager to update Edge driver...")
        try:
            subprocess.run(
                [manager_path, "--browser", "edge"],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=False
            )
        except Exception as e:
            print(f"[WARNING] Failed to run selenium-manager.exe: {e}")
    else:
        print("[INFO] selenium-manager.exe not found in cwd, skipping update.")

    # --- Step 2: 自动查找默认缓存目录里的最新 driver ---
    base = os.path.join(
        os.path.expanduser("~"),
        ".cache", "selenium", "msedgedriver", "win64"
    )

    versions = glob.glob(os.path.join(base, "*"))

    if not versions:
        raise FileNotFoundError("ERROR: No EdgeDriver found. Run selenium-manager.exe manually once.")

    versions.sort(reverse=True)
    latest_version_dir = versions[0]

    driver_path = os.path.join(latest_version_dir, "msedgedriver.exe")

    if not os.path.isfile(driver_path):
        raise FileNotFoundError(f"ERROR: Driver file missing: {driver_path}")

    print(f"[INFO] Using EdgeDriver: {driver_path}")
    return driver_path


if __name__ == "__main__":
    sock, thread = start_heart_monitor(udp_port)
    print("Heart monitor started. Press Ctrl+C to exit.")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Exiting...")

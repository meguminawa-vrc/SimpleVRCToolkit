#!D:\Codes\Anaconda3\envs\VRChat\python.exe
import sys
import socket
import argparse
import time
from zeroconf import Zeroconf, ServiceBrowser, ServiceListener

class OscListener(ServiceListener):
    def __init__(self):
        self.info = None

    def add_service(self, zeroconf, service_type, name):
        info = zeroconf.get_service_info(service_type, name)
        if info:
            self.info = info

def discover_osc_ip(timeout=5):
    """
    在局域网中发现 VRChat 的 OSC 服务（_osc._udp.local），返回 (ip, port)，无结果则返回 (None, None)。
    """
    zeroconf = Zeroconf()
    listener = OscListener()
    browser = ServiceBrowser(zeroconf, "_osc._udp.local.", listener)

    start = time.time()
    while time.time() - start < timeout:
        if listener.info:
            info = listener.info
            addrs = info.parsed_addresses()
            if addrs:
                ip = addrs[0]
                port = info.port
                zeroconf.close()
                return ip, port
        time.sleep(0.1)

    zeroconf.close()
    return None, None

def main():
    parser = argparse.ArgumentParser(description="Discover VRChat OSC endpoint.")
    parser.add_argument("timeout", nargs="?", type=float, default=5.0,
                        help="Timeout in seconds (default: 5.0)")
    args = parser.parse_args()

    try:
        ip, port = discover_osc_ip(timeout=args.timeout)
        if ip:
            print(f"{ip}:{port}")
            sys.exit(0)
        else:
            # 可选：打印空或特定提示
            print("", end="")
            sys.exit(1)
    except Exception as e:
        print(f"", end="")
        sys.exit(1)

if __name__ == "__main__":
    main()

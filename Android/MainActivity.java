package com.example.udpserverapp;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.os.Bundle;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;

public class MainActivity extends AppCompatActivity {

    private TextView tv;
    private BroadcastReceiver udpReceiver;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        tv = new TextView(this);
        tv.setText("UDP 服务正在运行，等待消息...\nIP: " + getLocalIpAddress() + ":9999");
        setContentView(tv);

        // 启动 UDP 服务
        Intent intent = new Intent(this, UdpServerService.class);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            startForegroundService(intent);
        } else {
            startService(intent);
        }

        // 注册广播接收器
        udpReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String msg = intent.getStringExtra("message");
                tv.setText("收到消息: " + msg + "\nIP: " + getLocalIpAddress() + ":9999");
            }
        };
        LocalBroadcastManager.getInstance(this).registerReceiver(
                udpReceiver, new IntentFilter("UDP_MESSAGE"));
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        LocalBroadcastManager.getInstance(this).unregisterReceiver(udpReceiver);
    }

    private String getLocalIpAddress() {
        try {
            Enumeration<NetworkInterface> interfaces = NetworkInterface.getNetworkInterfaces();
            while (interfaces.hasMoreElements()) {
                NetworkInterface intf = interfaces.nextElement();
                Enumeration<InetAddress> addrs = intf.getInetAddresses();
                while (addrs.hasMoreElements()) {
                    InetAddress addr = addrs.nextElement();
                    if (!addr.isLoopbackAddress() && addr.getHostAddress().indexOf(':') < 0) {
                        return addr.getHostAddress();
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return "0.0.0.0";
    }
}
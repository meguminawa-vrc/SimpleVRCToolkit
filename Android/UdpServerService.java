package com.example.udpserverapp;

import android.annotation.SuppressLint;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;

import androidx.core.app.NotificationCompat;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import org.json.JSONObject;

import java.net.DatagramPacket;
import java.net.DatagramSocket;

public class UdpServerService extends Service {
    private static final String TAG = "UdpServerService";
    private boolean running = true;
    private final int PORT = 9999;

    @SuppressLint("ForegroundServiceType")
    @Override
    public void onCreate() {
        super.onCreate();

        // 创建前台通知
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                    "udp_channel", "UDP Server", NotificationManager.IMPORTANCE_LOW);
            NotificationManager manager = getSystemService(NotificationManager.class);
            manager.createNotificationChannel(channel);
        }

        NotificationCompat.Builder builder = new NotificationCompat.Builder(this, "udp_channel")
                .setContentTitle("UDP Server")
                .setContentText("服务正在运行")
                .setSmallIcon(R.mipmap.ic_udp); // 替换为你的图标

        startForeground(1, builder.build());

        // 启动 UDP 服务线程
        new Thread(this::startUdpServer).start();
    }

    private void startUdpServer() {
        try {
            DatagramSocket socket = new DatagramSocket(PORT);
            byte[] buffer = new byte[4096];
            Log.d(TAG, "UDP Server started on port " + PORT);

            while (running) {
                try {
                    DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                    socket.receive(packet);

                    String msg = new String(packet.getData(), 0, packet.getLength());
                    Log.d(TAG, "收到消息: " + msg);

                    // 发送广播通知 Activity
                    Intent intent = new Intent("UDP_MESSAGE");
                    intent.putExtra("message", msg);
                    LocalBroadcastManager.getInstance(this).sendBroadcast(intent);

                    // JSON 解析与响应
                    try {
                        JSONObject json = new JSONObject(msg);
                        JSONObject responseJson = new JSONObject();
                        if (json.has("need_data")) {
                            String[] fields = json.getString("need_data").split(",");
                            for (String field : fields) {
                                field = field.trim();
                                if (field.equalsIgnoreCase("battery")) {
                                    responseJson.put("battery", getBatteryLevel());
                                } else if (field.equalsIgnoreCase("charge_state")) {
                                    responseJson.put("charge_state", getChargeState());
                                }
                            }
                        }

                        byte[] replyData = (responseJson.length() > 0 ? responseJson.toString() : "收到: " + msg).getBytes();
                        DatagramPacket replyPacket = new DatagramPacket(replyData, replyData.length, packet.getAddress(), packet.getPort());
                        socket.send(replyPacket);

                    } catch (Exception e) {
                        Log.e(TAG, "JSON 解析失败，发送普通回复", e);
                        sendErrorBroadcast("JSON 解析失败: " + e.getMessage());
                    }

                } catch (Exception e) {
                    Log.e(TAG, "UDP 接收或发送出错", e);
                    sendErrorBroadcast("UDP 错误: " + e.getMessage());
                }
            }

            socket.close();
        } catch (Exception e) {
            Log.e(TAG, "UDP Socket 创建失败", e);
            sendErrorBroadcast("UDP 服务启动失败: " + e.getMessage());
        }
    }
    // 发送错误信息到 Activity
    private void sendErrorBroadcast(String msg) {
        Intent errorIntent = new Intent("UDP_MESSAGE");
        errorIntent.putExtra("message", "[错误] " + msg);
        LocalBroadcastManager.getInstance(this).sendBroadcast(errorIntent);
    }
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        running = false;
        super.onDestroy();
    }

    /** 获取电池剩余百分比 */
    private int getBatteryLevel() {
        Intent batteryStatus = registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
        if (batteryStatus != null) {
            return batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
        }
        return -1;
    }

    /** 获取充电状态：0=未充电，1=正在充电 */
    private int getChargeState() {
        try {
            Intent batteryStatus = registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
            if (batteryStatus != null) {
                int status = batteryStatus.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
                if (status == BatteryManager.BATTERY_STATUS_CHARGING ||
                        status == BatteryManager.BATTERY_STATUS_FULL) {
                    return 1;
                } else {
                    return 0;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return -1;
    }
}

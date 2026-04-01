/*
 * ESP32-CAM 智能台灯 - 舵机控制系统
 * 
 * 硬件：
 *   主控: ESP32-CAM (AI-Thinker)
 *   舵机 x4 (详见 servo.h)
 *   WS2812 x24 (详见 ws2812.h)
 *   摄像头 (暂未启用)
 *
 * API (ESP32 Arduino SDK 3.0+):
 *   GET  /          - API 说明页面
 *   GET  /status    - 获取所有舵机当前角度
 *   POST /control   - 控制舵机 (JSON)
 *   POST /reset     - 所有舵机归零
 *   GET  /led       - 获取灯光状态
 *   POST /led       - 控制灯光 (JSON)
 *   GET  /actions   - 获取动作列表和播放状态
 *   POST /action    - 执行预设动作 (JSON)
 *   POST /action/stop - 停止当前动作
 */

#include <WiFi.h>
#include "http_routes.h"

// ============================================================
//  配置区
// ============================================================

// WiFi STA 模式配置
const char* WIFI_SSID     = "XcHome";
const char* WIFI_PASSWORD = "Cui123456";

// ============================================================
//  setup & loop
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n========================================");
  Serial.println("  ESP32 智能台灯 - 舵机控制系统");
  Serial.println("========================================");

  // 初始化舵机
  initServos();

  // 初始化 WS2812 灯带
  initLeds();

  // 连接 WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("\n[WiFi] 正在连接 %s", WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\n[WiFi] 已连接  IP: %s\n", WiFi.localIP().toString().c_str());

  // 注册路由并启动 HTTP 服务
  initServer();
}

void loop() {
  server.handleClient();
  updateLedEffect();
  updateServos();
  updateAction();
}
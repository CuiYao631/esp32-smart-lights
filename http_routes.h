#ifndef HTTP_ROUTES_H
#define HTTP_ROUTES_H

#include <WebServer.h>
#include <ArduinoJson.h>
#include "servo.h"
#include "ws2812.h"
#include "actions.h"

// ============================================================
//  全局 Web 服务器实例
// ============================================================

WebServer server(80);

// ============================================================
//  HTTP 路由处理
// ============================================================

// GET / — API 使用说明
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 智能台灯</title>
<style>
  *{box-sizing:border-box;margin:0;padding:0}
  body{font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;background:#0f172a;color:#e2e8f0;line-height:1.6}
  .wrap{max-width:860px;margin:0 auto;padding:24px 20px}
  h1{font-size:1.8em;margin-bottom:4px;color:#f8fafc}
  .sub{color:#94a3b8;margin-bottom:32px;font-size:.95em}
  .section{margin-bottom:36px}
  .section h2{font-size:1.15em;color:#38bdf8;border-bottom:1px solid #1e293b;padding-bottom:8px;margin-bottom:16px}
  .card{background:#1e293b;border-radius:10px;padding:18px 20px;margin-bottom:14px;border:1px solid #334155}
  .card:hover{border-color:#38bdf8;transition:border-color .2s}
  .method{display:inline-block;font-size:.75em;font-weight:700;padding:3px 8px;border-radius:4px;margin-right:8px;vertical-align:middle}
  .get{background:#059669;color:#fff}
  .post{background:#d97706;color:#fff}
  .path{font-family:monospace;font-size:1.05em;font-weight:600;color:#f1f5f9;vertical-align:middle}
  .desc{color:#94a3b8;margin-top:8px;font-size:.9em}
  pre{background:#0f172a;color:#4ade80;padding:14px 16px;border-radius:8px;overflow-x:auto;font-size:.85em;margin-top:10px;border:1px solid #334155}
  code{background:#334155;padding:2px 6px;border-radius:4px;font-size:.88em;color:#f472b6}
  table{width:100%;border-collapse:collapse;margin-top:10px;font-size:.88em}
  th{text-align:left;color:#38bdf8;padding:6px 10px;border-bottom:1px solid #334155}
  td{padding:6px 10px;border-bottom:1px solid #1e293b;color:#cbd5e1}
  td code{background:#334155;color:#fbbf24}
  .tag{display:inline-block;font-size:.7em;padding:2px 8px;border-radius:10px;margin-left:6px;vertical-align:middle}
  .tag-opt{background:#1e3a5f;color:#7dd3fc}
  .tag-req{background:#5f1e1e;color:#fca5a5}
  .hw{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:12px;margin-top:10px}
  .hw-item{background:#0f172a;border:1px solid #334155;border-radius:8px;padding:12px 14px;text-align:center}
  .hw-item .icon{font-size:1.6em;margin-bottom:4px}
  .hw-item .label{font-size:.82em;color:#94a3b8}
  .hw-item .val{font-weight:600;color:#f1f5f9;font-size:.95em}
  .footer{text-align:center;color:#475569;font-size:.78em;margin-top:40px;padding-top:16px;border-top:1px solid #1e293b}
</style></head><body>
<div class="wrap">

<h1>💡 ESP32 智能台灯</h1>
<p class="sub">ESP32-CAM · 4 舵机 · 24 WS2812 LED · RESTful API</p>

<!-- 硬件概览 -->
<div class="section">
<h2>🔌 硬件概览</h2>
<div class="hw">
  <div class="hw-item"><div class="icon">🎯</div><div class="val">ESP32-CAM</div><div class="label">主控</div></div>
  <div class="hw-item"><div class="icon">⚙️</div><div class="val">4 × 舵机</div><div class="label">GPIO 18/13/14/15</div></div>
  <div class="hw-item"><div class="icon">💡</div><div class="val">24 × WS2812</div><div class="label">GPIO 2</div></div>
  <div class="hw-item"><div class="icon">📷</div><div class="val">摄像头</div><div class="label">暂未启用</div></div>
</div>
</div>

<!-- 舵机 API -->
<div class="section">
<h2>⚙️ 舵机控制</h2>

<div class="card">
  <span class="method get">GET</span><span class="path">/status</span>
  <div class="desc">返回四个舵机的实时角度</div>
<pre>{
  "servos": [
    {"id":1, "name":"servo1", "angle":90, "pin":18},
    {"id":2, "name":"servo2", "angle":90, "pin":13},
    {"id":3, "name":"servo3", "angle":90, "pin":14},
    {"id":4, "name":"servo4", "angle":90, "pin":15}
  ]
}</pre>
</div>

<div class="card">
  <span class="method post">POST</span><span class="path">/control</span>
  <div class="desc">控制单个或多个舵机，只需传要操作的</div>
  <table>
    <tr><th>参数</th><th>类型</th><th>说明</th></tr>
    <tr><td><code>mode</code></td><td>string</td><td><code>relative</code>(默认, 相对旋转) 或 <code>absolute</code>(绝对角度)</td></tr>
    <tr><td><code>servo1</code>~<code>servo4</code></td><td>int</td><td>角度值，relative 时正负代表方向，absolute 为 0°~180°</td></tr>
  </table>
<pre>// 相对旋转：servo1 正转30°，servo2 反转20°
{"mode":"relative", "servo1":30, "servo2":-20}

// 绝对角度：servo1 转到 90°，servo3 转到 45°
{"mode":"absolute", "servo1":90, "servo3":45}</pre>
</div>

<div class="card">
  <span class="method post">POST</span><span class="path">/reset</span>
  <div class="desc">所有舵机归零 (0°)，无需请求体</div>
</div>
</div>

<!-- 灯光 API -->
<div class="section">
<h2>💡 灯光控制</h2>

<div class="card">
  <span class="method get">GET</span><span class="path">/led</span>
  <div class="desc">返回灯光当前状态</div>
<pre>{
  "mode": "solid",
  "brightness": 128,
  "color": {"r":255, "g":255, "b":255},
  "led_count": 24,
  "pin": 2
}</pre>
</div>

<div class="card">
  <span class="method post">POST</span><span class="path">/led</span>
  <div class="desc">控制灯光，所有字段均可选，只传需要修改的</div>
  <table>
    <tr><th>参数</th><th>类型</th><th>说明</th></tr>
    <tr><td><code>mode</code></td><td>string</td><td>灯效模式（见下表）</td></tr>
    <tr><td><code>color</code></td><td>object</td><td><code>{"r":0~255,"g":0~255,"b":0~255}</code> solid/breathing 有效</td></tr>
    <tr><td><code>brightness</code></td><td>int</td><td>全局亮度 0~255</td></tr>
  </table>
  <table>
    <tr><th>mode 值</th><th>效果</th></tr>
    <tr><td><code>off</code></td><td>关灯</td></tr>
    <tr><td><code>solid</code></td><td>纯色常亮</td></tr>
    <tr><td><code>breathing</code></td><td>呼吸灯（当前颜色渐明渐暗）</td></tr>
    <tr><td><code>rainbow</code></td><td>彩虹流动</td></tr>
    <tr><td><code>rainbow_cycle</code></td><td>彩虹循环</td></tr>
  </table>
<pre>// 暖白常亮，亮度200
{"mode":"solid", "color":{"r":255,"g":200,"b":120}, "brightness":200}

// 彩虹呼吸
{"mode":"breathing", "color":{"r":0,"g":128,"b":255}}</pre>
</div>
</div>

<!-- 动作 API -->
<div class="section">
<h2>🎭 预设动作</h2>

<div class="card">
  <span class="method get">GET</span><span class="path">/actions</span>
  <div class="desc">返回可用动作列表及当前播放状态</div>
<pre>{
  "playing": false,
  "available": [
    {"name":"nod",     "frames":5},
    {"name":"shake",   "frames":5},
    {"name":"wave",    "frames":7},
    {"name":"dance",   "frames":12},
    {"name":"stretch", "frames":5},
    {"name":"curious", "frames":8}
  ]
}</pre>
</div>

<div class="card">
  <span class="method post">POST</span><span class="path">/action</span>
  <div class="desc">执行预设动作</div>
  <table>
    <tr><th>参数</th><th>类型</th><th>说明</th></tr>
    <tr><td><code>name</code><span class="tag tag-req">必填</span></td><td>string</td><td>动作名称</td></tr>
    <tr><td><code>loops</code><span class="tag tag-opt">可选</span></td><td>int</td><td>循环次数，默认 1</td></tr>
  </table>
  <table>
    <tr><th>name 值</th><th>描述</th></tr>
    <tr><td><code>nod</code></td><td>🙂 点头</td></tr>
    <tr><td><code>shake</code></td><td>🙅 摇头</td></tr>
    <tr><td><code>wave</code></td><td>👋 打招呼</td></tr>
    <tr><td><code>dance</code></td><td>💃 跳舞（全身律动）</td></tr>
    <tr><td><code>stretch</code></td><td>🙆 伸懒腰</td></tr>
    <tr><td><code>curious</code></td><td>🔍 好奇张望</td></tr>
  </table>
<pre>{"name":"dance", "loops":2}</pre>
</div>

<div class="card">
  <span class="method post">POST</span><span class="path">/action/stop</span>
  <div class="desc">停止当前正在播放的动作，无需请求体</div>
</div>
</div>

<div class="footer">ESP32 智能台灯 · Powered by ESP32-CAM + Arduino</div>
</div>
</body></html>
)rawliteral";
  server.send(200, "text/html", html);
}

// GET /status — 返回舵机状态
void handleStatus() {
  server.send(200, "application/json", buildServoStatusJson());
}

// POST /control — 控制舵机
void handleControl() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"缺少JSON数据\"}");
    return;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  if (err) {
    String msg = "{\"error\":\"JSON解析失败: ";
    msg += err.c_str();
    msg += "\"}";
    server.send(400, "application/json", msg);
    return;
  }

  String mode = doc["mode"] | "relative";
  const char* keys[4] = {"servo1", "servo2", "servo3", "servo4"};

  bool anyChanged = false;
  for (int i = 0; i < 4; i++) {
    if (doc[keys[i]].is<int>()) {
      int value = doc[keys[i]].as<int>();
      if (mode == "absolute") {
        setServoAngle(i, value);
      } else {
        rotateServo(i, value);
      }
      anyChanged = true;
    }
  }

  if (!anyChanged) {
    server.send(400, "application/json", "{\"error\":\"未指定任何舵机参数(servo1~servo4)\"}");
    return;
  }

  server.send(200, "application/json", buildServoStatusJson());
}

// POST /reset — 归零
void handleReset() {
  resetAllServos();
  server.send(200, "application/json", buildServoStatusJson());
}

// GET /led — 灯光状态
void handleLedStatus() {
  server.send(200, "application/json", buildLedStatusJson());
}

// POST /led — 控制灯光
void handleLedControl() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"缺少JSON数据\"}");
    return;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  if (err) {
    String msg = "{\"error\":\"JSON解析失败: ";
    msg += err.c_str();
    msg += "\"}";
    server.send(400, "application/json", msg);
    return;
  }

  // 模式
  if (doc["mode"].is<const char*>()) {
    if (!setLedMode(doc["mode"].as<const char*>())) {
      server.send(400, "application/json", "{\"error\":\"未知灯效模式\"}");
      return;
    }
  }

  // 颜色
  if (doc["color"].is<JsonObject>()) {
    JsonObject c = doc["color"].as<JsonObject>();
    uint8_t r = c["r"] | ledR;
    uint8_t g = c["g"] | ledG;
    uint8_t b = c["b"] | ledB;
    setLedColor(r, g, b);
  }

  // 亮度
  if (doc["brightness"].is<int>()) {
    setLedBrightness(constrain(doc["brightness"].as<int>(), 0, 255));
  }

  server.send(200, "application/json", buildLedStatusJson());
}

// GET /actions — 动作列表
void handleActionsList() {
  server.send(200, "application/json", buildActionsJson());
}

// POST /action — 执行动作
void handleActionPlay() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"缺少JSON数据\"}");
    return;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  if (err) {
    String msg = "{\"error\":\"JSON解析失败: ";
    msg += err.c_str();
    msg += "\"}";
    server.send(400, "application/json", msg);
    return;
  }

  const char* name = doc["name"] | "";
  int loops = doc["loops"] | 1;
  if (strlen(name) == 0) {
    server.send(400, "application/json", "{\"error\":\"缺少 name 参数\"}");
    return;
  }

  if (!playAction(name, loops)) {
    server.send(400, "application/json", "{\"error\":\"未知动作名称\"}");
    return;
  }

  server.send(200, "application/json", buildActionsJson());
}

// POST /action/stop — 停止动作
void handleActionStop() {
  stopAction();
  server.send(200, "application/json", buildActionsJson());
}

// ============================================================
//  注册路由并启动服务器
// ============================================================

void initServer() {
  server.on("/",        HTTP_GET,  handleRoot);
  server.on("/status",  HTTP_GET,  handleStatus);
  server.on("/control", HTTP_POST, handleControl);
  server.on("/reset",   HTTP_POST, handleReset);
  server.on("/led",     HTTP_GET,  handleLedStatus);
  server.on("/led",     HTTP_POST, handleLedControl);
  server.on("/actions",    HTTP_GET,  handleActionsList);
  server.on("/action",     HTTP_POST, handleActionPlay);
  server.on("/action/stop", HTTP_POST, handleActionStop);

  server.begin();
  Serial.println("[HTTP] 服务已启动，端口 80\n");
}

#endif

#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>
#include <ArduinoJson.h>

// ============================================================
//  舵机配置
// ============================================================

#define SERVO_COUNT 4

// 舵机引脚 (ESP32-CAM 可用 GPIO, SD卡接口复用)
const int SERVO_PINS[SERVO_COUNT] = {18, 13, 14, 15};

// 舵机名称
const char* SERVO_NAMES[SERVO_COUNT] = {
  "servo1",   // 1号 - 灯头左右
  "servo2",   // 2号 - 上下点头
  "servo3",   // 3号 - 力臂关节
  "servo4"    // 4号 - 底座
};

// 舵机 PWM 参数 (50Hz, 14-bit)
const int SERVO_FREQ       = 50;
const int SERVO_RESOLUTION = 14;    // 16384 级
const int DUTY_MIN         = 410;   // 0°   → 0.5ms
const int DUTY_MAX         = 2048;  // 180° → 2.5ms

// 角度范围
const int ANGLE_MIN = 0;
const int ANGLE_MAX = 180;

// ============================================================
//  全局状态
// ============================================================

int servoAngles[SERVO_COUNT] = {0, 0, 0, 0};

// ============================================================
//  舵机控制函数
// ============================================================

// 角度 → PWM 占空比
int angleToDuty(int angle) {
  angle = constrain(angle, ANGLE_MIN, ANGLE_MAX);
  return map(angle, 0, 180, DUTY_MIN, DUTY_MAX);
}

// 设置舵机到绝对角度
void setServoAngle(int idx, int angle) {
  if (idx < 0 || idx >= SERVO_COUNT) return;
  angle = constrain(angle, ANGLE_MIN, ANGLE_MAX);
  servoAngles[idx] = angle;
  ledcWrite(SERVO_PINS[idx], angleToDuty(angle));
  Serial.printf("[舵机%d %s] → %d°\n", idx + 1, SERVO_NAMES[idx], angle);
}

// 相对旋转 (正值正转, 负值反转)
void rotateServo(int idx, int degrees) {
  if (idx < 0 || idx >= SERVO_COUNT) return;
  int newAngle = constrain(servoAngles[idx] + degrees, ANGLE_MIN, ANGLE_MAX);
  setServoAngle(idx, newAngle);
}

// 所有舵机归零
void resetAllServos() {
  for (int i = 0; i < SERVO_COUNT; i++) {
    setServoAngle(i, 0);
  }
  Serial.println("[系统] 所有舵机已归零");
}

// 初始化所有舵机 LEDC 通道
void initServos() {
  for (int i = 0; i < SERVO_COUNT; i++) {
    if (!ledcAttach(SERVO_PINS[i], SERVO_FREQ, SERVO_RESOLUTION)) {
      Serial.printf("[错误] 舵机%d (GPIO %d) LEDC 初始化失败!\n", i + 1, SERVO_PINS[i]);
    } else {
      Serial.printf("[OK]   舵机%d (GPIO %d) %s 就绪\n", i + 1, SERVO_PINS[i], SERVO_NAMES[i]);
    }
  }
  resetAllServos();
}

// 构造 JSON 状态响应
String buildServoStatusJson() {
  JsonDocument doc;
  JsonArray arr = doc["servos"].to<JsonArray>();
  for (int i = 0; i < SERVO_COUNT; i++) {
    JsonObject s = arr.add<JsonObject>();
    s["id"]    = i + 1;
    s["name"]  = SERVO_NAMES[i];
    s["angle"] = servoAngles[i];
    s["pin"]   = SERVO_PINS[i];
  }
  String out;
  serializeJsonPretty(doc, out);
  return out;
}

#endif

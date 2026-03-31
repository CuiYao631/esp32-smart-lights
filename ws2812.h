#ifndef WS2812_H
#define WS2812_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

// ============================================================
//  WS2812 配置
// ============================================================

#define LED_PIN   2       // ESP32-CAM GPIO2 (SD卡未使用时可用)
#define LED_COUNT 24

// 灯效模式枚举
enum LedMode {
  LED_OFF = 0,      // 关灯
  LED_SOLID,        // 纯色
  LED_BREATHING,    // 呼吸灯
  LED_RAINBOW,      // 彩虹
  LED_RAINBOW_CYCLE // 彩虹循环
};

const char* LED_MODE_NAMES[] = {
  "off", "solid", "breathing", "rainbow", "rainbow_cycle"
};
const int LED_MODE_COUNT = 5;

// ============================================================
//  全局状态
// ============================================================

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

LedMode  ledMode       = LED_OFF;
uint8_t  ledBrightness = 128;       // 0~255
uint8_t  ledR = 255, ledG = 255, ledB = 255;  // 当前颜色
unsigned long ledLastUpdate = 0;    // 灯效帧计时
int      ledAnimStep   = 0;        // 动画步进

// ============================================================
//  内部工具
// ============================================================

// 色轮 (0~255 映射到彩虹色)
uint32_t colorWheel(byte pos) {
  pos = 255 - pos;
  if (pos < 85) {
    return strip.Color(255 - pos * 3, 0, pos * 3);
  } else if (pos < 170) {
    pos -= 85;
    return strip.Color(0, pos * 3, 255 - pos * 3);
  } else {
    pos -= 170;
    return strip.Color(pos * 3, 255 - pos * 3, 0);
  }
}

// ============================================================
//  灯效更新 (非阻塞, 在 loop 中调用)
// ============================================================

void updateLedEffect() {
  unsigned long now = millis();

  switch (ledMode) {

    case LED_OFF:
      if (ledLastUpdate == 0) {   // 仅执行一次
        strip.clear();
        strip.show();
        ledLastUpdate = 1;
      }
      break;

    case LED_SOLID:
      if (ledLastUpdate == 0) {
        strip.fill(strip.Color(ledR, ledG, ledB));
        strip.show();
        ledLastUpdate = 1;
      }
      break;

    case LED_BREATHING:
      if (now - ledLastUpdate >= 20) {
        ledLastUpdate = now;
        // 正弦呼吸曲线
        float phase = (ledAnimStep % 256) / 255.0 * PI * 2;
        float factor = (sin(phase) + 1.0) / 2.0;  // 0.0 ~ 1.0
        uint8_t r = ledR * factor;
        uint8_t g = ledG * factor;
        uint8_t b = ledB * factor;
        strip.fill(strip.Color(r, g, b));
        strip.show();
        ledAnimStep++;
      }
      break;

    case LED_RAINBOW:
      if (now - ledLastUpdate >= 20) {
        ledLastUpdate = now;
        for (int i = 0; i < LED_COUNT; i++) {
          strip.setPixelColor(i, colorWheel((i * 256 / LED_COUNT + ledAnimStep) & 255));
        }
        strip.show();
        ledAnimStep++;
      }
      break;

    case LED_RAINBOW_CYCLE:
      if (now - ledLastUpdate >= 10) {
        ledLastUpdate = now;
        for (int i = 0; i < LED_COUNT; i++) {
          strip.setPixelColor(i, colorWheel(((i * 256 / LED_COUNT) + ledAnimStep) & 255));
        }
        strip.show();
        ledAnimStep = (ledAnimStep + 1) % (256 * 5);
      }
      break;
  }
}

// ============================================================
//  公开控制函数
// ============================================================

// 初始化 WS2812
void initLeds() {
  strip.begin();
  strip.setBrightness(ledBrightness);
  strip.clear();
  strip.show();
  Serial.printf("[OK]   WS2812 x%d (GPIO %d) 就绪\n", LED_COUNT, LED_PIN);
}

// 强制刷新 (模式切换时重置计时器)
void ledMarkDirty() {
  ledLastUpdate = 0;
  ledAnimStep   = 0;
}

// 设置模式 (字符串)
bool setLedMode(const char* modeStr) {
  for (int i = 0; i < LED_MODE_COUNT; i++) {
    if (strcmp(modeStr, LED_MODE_NAMES[i]) == 0) {
      ledMode = (LedMode)i;
      ledMarkDirty();
      Serial.printf("[灯光] 模式 → %s\n", modeStr);
      return true;
    }
  }
  return false;
}

// 设置纯色
void setLedColor(uint8_t r, uint8_t g, uint8_t b) {
  ledR = r; ledG = g; ledB = b;
  ledMarkDirty();
  Serial.printf("[灯光] 颜色 → R%d G%d B%d\n", r, g, b);
}

// 设置亮度
void setLedBrightness(uint8_t brightness) {
  ledBrightness = brightness;
  strip.setBrightness(ledBrightness);
  strip.show();
  Serial.printf("[灯光] 亮度 → %d\n", brightness);
}

// 构造 JSON 状态
String buildLedStatusJson() {
  JsonDocument doc;
  doc["mode"]       = LED_MODE_NAMES[ledMode];
  doc["brightness"] = ledBrightness;
  JsonObject color  = doc["color"].to<JsonObject>();
  color["r"] = ledR;
  color["g"] = ledG;
  color["b"] = ledB;
  doc["led_count"]  = LED_COUNT;
  doc["pin"]        = LED_PIN;
  String out;
  serializeJsonPretty(doc, out);
  return out;
}

#endif

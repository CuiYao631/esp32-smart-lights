#ifndef ACTIONS_H
#define ACTIONS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "servo.h"

// ============================================================
//  动作关键帧系统 (非阻塞)
// ============================================================

// 单个关键帧: 四个舵机目标角度 + 过渡时间(ms)
struct Keyframe {
  int angles[SERVO_COUNT];  // -1 表示不动
  int duration;             // 停留时间 ms
};

// 动作定义
struct Action {
  const char*    name;
  const Keyframe* frames;
  int            frameCount;
  int            loopCount;   // 循环次数, 0=不循环
};

// ============================================================
//  预设关键帧
// ============================================================

// --- 点头 ---
const Keyframe FRAMES_NOD[] = {
  // servo1  servo2  servo3  servo4  duration
  {{  90,     60,     90,     90  },  300},
  {{  90,    120,     90,     90  },  300},
  {{  90,     60,     90,     90  },  300},
  {{  90,    120,     90,     90  },  300},
  {{  90,     90,     90,     90  },  400},
};

// --- 摇头 ---
const Keyframe FRAMES_SHAKE[] = {
  {{  45,     90,     90,     90  },  300},
  {{ 135,     90,     90,     90  },  300},
  {{  45,     90,     90,     90  },  300},
  {{ 135,     90,     90,     90  },  300},
  {{  90,     90,     90,     90  },  400},
};

// --- 打招呼 ---
const Keyframe FRAMES_WAVE[] = {
  {{  90,    110,    130,     90  },  400},
  {{ 120,    110,    130,     90  },  250},
  {{  60,    110,    130,     90  },  250},
  {{ 120,    110,    130,     90  },  250},
  {{  60,    110,    130,     90  },  250},
  {{ 120,    110,    130,     90  },  250},
  {{  90,     90,     90,     90  },  400},
};

// --- 跳舞 ---
const Keyframe FRAMES_DANCE[] = {
  {{  60,    120,     60,     45  },  300},
  {{ 120,     60,    120,    135  },  300},
  {{  60,    120,     60,     45  },  300},
  {{ 120,     60,    120,    135  },  300},
  {{  45,     90,    150,     90  },  250},
  {{ 135,     90,     30,     90  },  250},
  {{  45,     90,    150,     90  },  250},
  {{ 135,     90,     30,     90  },  250},
  {{  90,     60,     90,     45  },  300},
  {{  90,    120,     90,    135  },  300},
  {{  90,     60,     90,     45  },  300},
  {{  90,     90,     90,     90  },  500},
};

// --- 伸懒腰 ---
const Keyframe FRAMES_STRETCH[] = {
  {{  90,     90,     30,     90  },  600},
  {{  90,     50,      0,     90  },  800},
  {{  70,     50,      0,     90  },  400},
  {{ 110,     50,      0,     90  },  400},
  {{  90,     90,     90,     90  },  600},
};

// --- 好奇张望 ---
const Keyframe FRAMES_CURIOUS[] = {
  {{  60,     70,    100,     60  },  500},
  {{  60,    100,    100,     60  },  300},
  {{  60,     70,    100,     60  },  300},
  {{ 120,     70,    100,    120  },  500},
  {{ 120,    100,    100,    120  },  300},
  {{ 120,     70,    100,    120  },  300},
  {{  90,     80,    110,     90  },  400},
  {{  90,     90,     90,     90  },  400},
};

// --- 撒娇 ---
const Keyframe FRAMES_CUTE[] = {
  {{  80,    100,    100,     80  },  200},
  {{ 100,    100,    100,    100  },  200},
  {{  80,    100,    100,     80  },  200},
  {{ 100,    100,    100,    100  },  200},
  {{  70,    110,     80,     70  },  300},
  {{ 110,    110,     80,    110  },  300},
  {{  70,    110,     80,     70  },  300},
  {{  90,     90,     90,     90  },  400},
};

// ============================================================
//  动作注册表
// ============================================================

#define ACTION_COUNT 6

const Action ACTIONS[ACTION_COUNT] = {
  {"nod",      FRAMES_NOD,      sizeof(FRAMES_NOD)     / sizeof(Keyframe), 0},
  {"shake",    FRAMES_SHAKE,    sizeof(FRAMES_SHAKE)   / sizeof(Keyframe), 0},
  {"wave",     FRAMES_WAVE,     sizeof(FRAMES_WAVE)    / sizeof(Keyframe), 0},
  {"dance",    FRAMES_DANCE,    sizeof(FRAMES_DANCE)   / sizeof(Keyframe), 0},
  {"stretch",  FRAMES_STRETCH,  sizeof(FRAMES_STRETCH) / sizeof(Keyframe), 0},
  {"curious",  FRAMES_CURIOUS,  sizeof(FRAMES_CURIOUS) / sizeof(Keyframe), 0},
};

// ============================================================
//  播放状态机
// ============================================================

bool          actionPlaying    = false;
const Action* actionCurrent    = nullptr;
int           actionFrameIdx   = 0;
int           actionLoopsLeft  = 0;
unsigned long actionFrameStart = 0;

// 开始播放动作
bool playAction(const char* name, int loops = 1) {
  for (int i = 0; i < ACTION_COUNT; i++) {
    if (strcmp(name, ACTIONS[i].name) == 0) {
      actionCurrent    = &ACTIONS[i];
      actionFrameIdx   = 0;
      actionLoopsLeft  = (loops > 0) ? loops : 1;
      actionPlaying    = true;
      actionFrameStart = 0; // 触发立即执行第一帧
      Serial.printf("[动作] 开始播放: %s (循环%d次)\n", name, actionLoopsLeft);
      return true;
    }
  }
  Serial.printf("[动作] 未找到: %s\n", name);
  return false;
}

// 停止播放
void stopAction() {
  if (actionPlaying) {
    actionPlaying = false;
    Serial.println("[动作] 已停止");
  }
}

// 非阻塞更新 (在 loop 中调用)
void updateAction() {
  if (!actionPlaying || !actionCurrent) return;

  unsigned long now = millis();

  // 首帧或时间到 → 执行下一帧
  if (actionFrameStart == 0 || (now - actionFrameStart >= (unsigned long)actionCurrent->frames[actionFrameIdx].duration)) {
    // 前进到下一帧
    if (actionFrameStart != 0) {
      actionFrameIdx++;
    }

    // 检查是否一轮播完
    if (actionFrameIdx >= actionCurrent->frameCount) {
      actionLoopsLeft--;
      if (actionLoopsLeft <= 0) {
        actionPlaying = false;
        Serial.println("[动作] 播放完毕");
        return;
      }
      actionFrameIdx = 0;
    }

    // 应用当前帧
    const Keyframe& kf = actionCurrent->frames[actionFrameIdx];
    for (int i = 0; i < SERVO_COUNT; i++) {
      if (kf.angles[i] >= 0) {
        setServoAngle(i, kf.angles[i]);
      }
    }
    actionFrameStart = now;
  }
}

// 构造动作列表 JSON
String buildActionsJson() {
  JsonDocument doc;
  doc["playing"] = actionPlaying;
  if (actionPlaying && actionCurrent) {
    doc["current"] = actionCurrent->name;
  }
  JsonArray arr = doc["available"].to<JsonArray>();
  for (int i = 0; i < ACTION_COUNT; i++) {
    JsonObject a = arr.add<JsonObject>();
    a["name"]   = ACTIONS[i].name;
    a["frames"] = ACTIONS[i].frameCount;
  }
  String out;
  serializeJsonPretty(doc, out);
  return out;
}

#endif

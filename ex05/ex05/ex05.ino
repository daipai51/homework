const int touchPin = 4;
const int ledPin = 2;

int speedLevel = 1;

bool isTouching = false;
bool lastIsTouching = false;
const int THRESHOLD = 500;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 80;

int brightness = 0;
int fadeAmount = 5;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  analogWrite(ledPin, 0);
  
  Serial.println("=================================");
  Serial.println("  多档位触摸调速呼吸灯");
  Serial.println("  触摸 GPIO4 切换速度档位");
  Serial.println("  1慢 → 2中 → 3快 → 1慢 ...");
  Serial.println("=================================");
  printCurrentStatus();
}

void loop() {
  // ===== 1. 读取触摸状态 =====
  int touchValue = touchRead(touchPin);
  isTouching = (touchValue < THRESHOLD);

  if (isTouching && !lastIsTouching) {
    // 检查防抖时间
    if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
      // ===== 有效触摸！切换档位 =====
      speedLevel++;
      if (speedLevel > 3) {
        speedLevel = 1;
      }
      
      // 串口输出切换信息
      Serial.println("┌──────────────────────────┐");
      Serial.print("│  ★ 触摸触发！切换至档位 ");
      Serial.print(speedLevel);
      Serial.print(" (");
      if (speedLevel == 1) Serial.print("慢速");
      else if (speedLevel == 2) Serial.print("中速");
      else if (speedLevel == 3) Serial.print("快速");
      Serial.println(")  │");
      Serial.println("└──────────────────────────┘");
      
      lastDebounceTime = millis();
    }
  }

  // ===== 3. 更新上一次触摸状态 =====
  lastIsTouching = isTouching;

  // ===== 4. 呼吸灯更新 =====
  brightness += fadeAmount;
  if (brightness >= 255 || brightness <= 0) {
    fadeAmount = -fadeAmount;
    if (brightness > 255) brightness = 255;
    if (brightness < 0) brightness = 0;
  }
  analogWrite(ledPin, brightness);

  // ===== 5. 根据档位控制速度 =====
  int delayTime;
  switch (speedLevel) {
    case 1:  delayTime = 50; break;   // 慢速：约5秒一个呼吸周期
    case 2:  delayTime = 20; break;   // 中速：约2秒一个呼吸周期
    case 3:  delayTime = 5;  break;   // 快速：约0.5秒一个呼吸周期
    default: delayTime = 20; break;
  }

  static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime > 1000) {
    lastPrintTime = millis();
    printCurrentStatus();
  }

  delay(delayTime);
}

void printCurrentStatus() {
  Serial.print("📊 档位: ");
  Serial.print(speedLevel);
  Serial.print(" (");
  if (speedLevel == 1) Serial.print("慢速");
  else if (speedLevel == 2) Serial.print("中速");
  else if (speedLevel == 3) Serial.print("快速");
  Serial.print(")  |  触摸状态: ");
  Serial.println(isTouching ? "✅ 触摸中" : "❌ 未触摸");
}
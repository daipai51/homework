const int touchPin = 4;
const int ledPin = 2;

bool ledState = false;          // LED状态
bool touchDetected = false;     // 是否检测到触摸
unsigned long lastTouchTime = 0;
const unsigned long debounceDelay = 100;

const int THRESHOLD = 500;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);    // 初始熄灭
  Serial.println("触摸自锁开关启动");
}

void loop() {
  int touchValue = touchRead(touchPin);
  

  bool isTouching = (touchValue < THRESHOLD);
  
  // 串口调试
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 300) {
    lastPrint = millis();
    Serial.print("数值:");
    Serial.print(touchValue);
    Serial.print("  触摸:");
    Serial.print(isTouching ? "Y" : "N");
    Serial.print("  LED:");
    Serial.println(ledState ? "ON" : "OFF");
  }

  // 边缘检测 + 防抖
  if (isTouching && !touchDetected) {
    // 检测到按下瞬间（从未触摸变为触摸）
    if (millis() - lastTouchTime > debounceDelay) {
      lastTouchTime = millis();
      
      // 翻转 LED 状态
      ledState = !ledState;
      digitalWrite(ledPin, ledState ? HIGH : LOW);
      
      Serial.println("=== 触摸触发！===");
      Serial.print("LED 状态: ");
      Serial.println(ledState ? "点亮" : "熄灭");
    }
  }
  
  // 更新触摸状态
  touchDetected = isTouching;
  
  delay(20);  // 小延迟，避免串口刷屏
}
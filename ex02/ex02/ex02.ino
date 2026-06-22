const int ledPin = 2;
unsigned long prevTime = 0; // 记录上次状态切换时间
const unsigned long interval = 500; // 500ms切换一次，周期1000ms=1Hz
bool ledState = LOW;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  unsigned long currTime = millis(); // 获取当前运行毫秒数
  // 判断是否到达切换间隔
  if(currTime - prevTime >= interval){
    prevTime = currTime; // 更新记录时间
    ledState = !ledState; // 翻转LED状态
    digitalWrite(ledPin, ledState);
    Serial.print("LED状态更新：");
    Serial.println(ledState ? "ON" : "OFF");
  }
}
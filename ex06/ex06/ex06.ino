const int ledAPin = 2;        // LED_A 引脚
const int ledBPin = 4;        // LED_B 引脚

int pwmValue = 0;              // 当前PWM值：0 → 255 → 0 循环
int step = 1;                  // 步长（值越小过渡越平滑细腻）

void setup() {
  Serial.begin(115200);
  
  // 初始化两个LED引脚为输出模式
  pinMode(ledAPin, OUTPUT);
  pinMode(ledBPin, OUTPUT);
  
  // 初始状态：LED_A熄灭，LED_B点亮
  analogWrite(ledAPin, 0);
  analogWrite(ledBPin, 255);
  
  Serial.println("=================================");
  Serial.println("  警车双闪灯效已启动");
  Serial.println("  LED_A (GPIO12) ↔ LED_B (GPIO13)");
  Serial.println("  反相PWM交替渐变闪烁");
  Serial.println("=================================");
}

void loop() {
  // ===== 1. 输出反相PWM =====
  // LED_A 亮度 = pwmValue (0→255渐亮)
  // LED_B 亮度 = 255 - pwmValue (255→0渐暗)
  // 两者始终保持反相关系
  analogWrite(ledAPin, pwmValue);
  analogWrite(ledBPin, 255 - pwmValue);

  // ===== 2. 更新PWM值（三角波） =====
  pwmValue += step;

  // 到达边界时反转方向
  if (pwmValue >= 255) {
    pwmValue = 255;
    step = -step;        // 改为递减
  } else if (pwmValue <= 0) {
    pwmValue = 0;
    step = -step;        // 改为递增
  }

  delay(10);

  static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime > 500) {
    lastPrintTime = millis();
    Serial.print("PWM: ");
    Serial.print(pwmValue);
    Serial.print("  |  LED_A: ");
    Serial.print(pwmValue);
    Serial.print("  |  LED_B: ");
    Serial.println(255 - pwmValue);
  }
}
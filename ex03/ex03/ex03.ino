const int ledPin = 2;

// 时间参数定义
const unsigned long shortOn = 200;    // 短亮时长 S
const unsigned long longOn = 600;     // 长亮时长 O
const unsigned long dotGap = 200;      // 单个闪烁间隔熄灭
const unsigned long wordGap = 2000;    // 整套SOS结束长停顿

// 状态机变量
unsigned long prevTime = 0;
int stage = 0; // 0:S短闪3次 1:O长闪3次 2:S短闪3次 3:长停顿
int cnt = 0;   // 当前阶段闪烁计数
bool ledState = LOW;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop() {
  unsigned long currTime = millis();
  if(currTime - prevTime < (ledState ? (stage==1 ? longOn : shortOn) : dotGap)){
    return; // 未到切换时间，直接退出循环
  }
  prevTime = currTime;
  ledState = !ledState;
  digitalWrite(ledPin, ledState);

  // LED熄灭时切换阶段逻辑
  if(!ledState){
    cnt++;
    switch(stage){
      case 0: // S 3短闪
        if(cnt >= 3){ stage=1; cnt=0; }
        break;
      case 1: // O 3长闪
        if(cnt >= 3){ stage=2; cnt=0; }
        break;
      case 2: // S 3短闪
        if(cnt >= 3){ stage=3; cnt=0; prevTime = currTime + wordGap; }
        break;
      case 3: // 长停顿结束，重置从头循环
        stage=0; cnt=0;
        break;
    }
  }
}
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// ========== 配置区域 ==========
const char* ap_ssid = "ESP32_dai";     // 热点名称
const char* ap_password = "12345678";    // 热点密码（至少8位）

// 引脚定义
const int TOUCH_PIN = T0;      // GPIO4 (触摸引脚)
const int LED_PIN = 2;          // 板载LED

// 触摸阈值
const int TOUCH_THRESHOLD = 500;  // 小于此值认为触摸

// ========== 全局变量 ==========
WebServer server(80);

// 系统状态
enum SystemState {
  STATE_DISARMED,    // 撤防状态
  STATE_ARMED,       // 布防状态
  STATE_ALARM        // 报警状态
};

SystemState currentState = STATE_DISARMED;
bool isTouched = false;          // 当前是否触摸
int touchValue = 0;              // 当前触摸值
unsigned long alarmStartTime = 0;
bool ledState = false;
unsigned long lastLedToggle = 0;
const int ALARM_BLINK_INTERVAL = 100; // 报警闪烁间隔(毫秒)

// ========== HTML页面 ==========
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 安防报警系统</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #0c0c1d, #1a1a2e);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            color: #fff;
            padding: 20px;
        }
        .container {
            background: rgba(255, 255, 255, 0.05);
            backdrop-filter: blur(20px);
            border-radius: 30px;
            padding: 35px 40px;
            box-shadow: 0 25px 50px rgba(0,0,0,0.5);
            border: 1px solid rgba(255,255,255,0.08);
            text-align: center;
            max-width: 480px;
            width: 100%;
        }
        h1 {
            font-size: 26px;
            margin-bottom: 5px;
            background: linear-gradient(90deg, #00d2ff, #3a7bd5);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        .subtitle {
            color: #8892b0;
            font-size: 12px;
            margin-bottom: 25px;
        }
        .info-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 12px;
            margin-bottom: 25px;
        }
        .info-card {
            background: rgba(0,0,0,0.3);
            border-radius: 12px;
            padding: 15px;
            border: 1px solid rgba(255,255,255,0.05);
        }
        .info-label {
            font-size: 11px;
            color: #8892b0;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        .info-value {
            font-size: 20px;
            font-weight: 700;
            margin-top: 4px;
        }
        .status-box {
            background: rgba(0,0,0,0.3);
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 25px;
            border: 2px solid rgba(255,255,255,0.05);
            transition: all 0.3s ease;
        }
        .status-label {
            font-size: 13px;
            color: #8892b0;
            text-transform: uppercase;
            letter-spacing: 2px;
        }
        .status-value {
            font-size: 30px;
            font-weight: 700;
            margin-top: 5px;
            transition: all 0.3s ease;
        }
        .status-disarmed .status-value {
            color: #4ade80;
        }
        .status-armed .status-value {
            color: #facc15;
        }
        .status-alarm .status-value {
            color: #f87171;
            animation: pulse 0.5s ease-in-out infinite alternate;
        }
        @keyframes pulse {
            from { opacity: 1; transform: scale(1); }
            to { opacity: 0.5; transform: scale(1.05); }
        }
        .btn-group {
            display: flex;
            gap: 12px;
            justify-content: center;
            flex-wrap: wrap;
        }
        .btn {
            padding: 12px 30px;
            border: none;
            border-radius: 50px;
            font-size: 15px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            flex: 1;
            min-width: 100px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        .btn-arm {
            background: linear-gradient(135deg, #facc15, #f59e0b);
            color: #1a1a2e;
        }
        .btn-arm:hover:not(:disabled) {
            transform: translateY(-2px);
            box-shadow: 0 10px 30px rgba(250, 204, 21, 0.3);
        }
        .btn-arm:disabled {
            opacity: 0.4;
            cursor: not-allowed;
            transform: none;
        }
        .btn-disarm {
            background: linear-gradient(135deg, #4ade80, #22c55e);
            color: #1a1a2e;
        }
        .btn-disarm:hover:not(:disabled) {
            transform: translateY(-2px);
            box-shadow: 0 10px 30px rgba(74, 222, 128, 0.3);
        }
        .btn-disarm:disabled {
            opacity: 0.4;
            cursor: not-allowed;
            transform: none;
        }
        .touch-display {
            background: rgba(0,0,0,0.2);
            border-radius: 10px;
            padding: 8px 15px;
            margin-top: 15px;
            font-size: 13px;
            color: #8892b0;
        }
        .touch-display span {
            color: #fff;
            font-weight: 600;
        }
        .status-dot {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
            vertical-align: middle;
        }
        .dot-safe { background: #4ade80; }
        .dot-touch { background: #f87171; animation: pulse 0.5s ease-in-out infinite alternate; }
        .footer {
            margin-top: 20px;
            font-size: 11px;
            color: #4a5568;
        }
        @media (max-width: 400px) {
            .container { padding: 20px; }
            .info-grid { grid-template-columns: 1fr; }
            .btn { min-width: 80px; font-size: 13px; padding: 10px 20px; }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔐 安防报警系统</h1>
        <p class="subtitle">ESP32 智能安防主机</p >
        
        <div class="info-grid">
            <div class="info-card">
                <div class="info-label">📊 触摸值</div>
                <div class="info-value" id="touchValue">--</div>
            </div>
            <div class="info-card">
                <div class="info-label">👆 触摸状态</div>
                <div class="info-value" id="touchStatus">--</div>
            </div>
        </div>

        <div class="status-box" id="statusBox">
            <div class="status-label">🔒 系统状态</div>
            <div class="status-value" id="systemStatus">--</div>
        </div>

        <div class="btn-group">
            <button class="btn btn-arm" id="btnArm" onclick="armSystem()">🔫 布防</button>
            <button class="btn btn-disarm" id="btnDisarm" onclick="disarmSystem()">🔓 撤防</button>
        </div>

        <div class="touch-display">
            <span id="touchIndicator">🔵 安全</span>
        </div>
        <div class="footer">ESP32 安防系统 v1.0</div>
    </div>

    <script>
        function updateStatus() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    // 更新触摸值
                    document.getElementById('touchValue').textContent = data.touchValue;
                    
                    // 更新触摸状态
                    const touchStatus = document.getElementById('touchStatus');
                    if (data.isTouched) {
                        touchStatus.innerHTML = '<span style="color:#f87171;">⚠️ 触摸</span>';
                    } else {
                        touchStatus.innerHTML = '<span style="color:#4ade80;">✅ 安全</span>';
                    }
                    
                    // 更新系统状态
                    const statusBox = document.getElementById('statusBox');
                    const systemStatus = document.getElementById('systemStatus');
                    const statusTexts = {
                        'DISARMED': '🔓 撤防',
                        'ARMED': '🔒 布防中',
                        'ALARM': '🚨 报警!!!'
                    };
                    systemStatus.textContent = statusTexts[data.state] || data.state;
                    
                    // 更新状态样式
                    statusBox.className = 'status-box';
                    if (data.state === 'DISARMED') {
                        statusBox.classList.add('status-disarmed');
                    } else if (data.state === 'ARMED') {
                        statusBox.classList.add('status-armed');
                    } else if (data.state === 'ALARM') {
                        statusBox.classList.add('status-alarm');
                    }
                    
                    // 更新触摸指示器
                    const indicator = document.getElementById('touchIndicator');
                    if (data.isTouched) {
                        indicator.innerHTML = '🔴 检测到触摸!';
                    } else {
                        indicator.innerHTML = '🟢 安全状态';
                    }
                    
                    // 更新按钮状态
                    document.getElementById('btnArm').disabled = (data.state === 'ARMED' || data.state === 'ALARM');
                    document.getElementById('btnDisarm').disabled = (data.state === 'DISARMED');
                })
                .catch(err => console.log('更新失败:', err));
        }

        function armSystem() {
            fetch('/api/arm', { method: 'POST' })
                .then(response => response.json())
                .then(data => {
                    if (data.success) updateStatus();
                })
                .catch(err => console.log('布防失败:', err));
        }

        function disarmSystem() {
            fetch('/api/disarm', { method: 'POST' })
                .then(response => response.json())
                .then(data => {
                    if (data.success) updateStatus();
                })
                .catch(err => console.log('撤防失败:', err));
        }

        // 每秒更新一次状态
        setInterval(updateStatus, 500);
        // 立即更新一次
        updateStatus();
    </script>
</body>
</html>
)rawliteral";

// ========== Web服务器路由处理 ==========

// 根路径 - 返回HTML页面
void handleRoot() {
  server.send(200, "text/html", index_html);
}

// API: 获取状态
void handleApiStatus() {
  String json = "{";
  json += "\"state\":\"";
  switch(currentState) {
    case STATE_DISARMED: json += "DISARMED"; break;
    case STATE_ARMED: json += "ARMED"; break;
    case STATE_ALARM: json += "ALARM"; break;
  }
  json += "\",";
  json += "\"touchValue\":" + String(touchValue) + ",";
  json += "\"isTouched\":" + String(isTouched ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

// API: 布防
void handleApiArm() {
  if (currentState == STATE_DISARMED) {
    currentState = STATE_ARMED;
    // 布防时重置LED
    ledState = false;
    digitalWrite(LED_PIN, LOW);
    Serial.println("🔒 系统已布防");
  }
  String json = "{\"success\":true,\"state\":\"ARMED\"}";
  server.send(200, "application/json", json);
}

// API: 撤防
void handleApiDisarm() {
  currentState = STATE_DISARMED;
  // 撤防时关闭LED
  ledState = false;
  digitalWrite(LED_PIN, LOW);
  Serial.println("🔓 系统已撤防");
  String json = "{\"success\":true,\"state\":\"DISARMED\"}";
  server.send(200, "application/json", json);
}

// ========== 初始化 ==========
void setup() {
  Serial.begin(115200);
  delay(100);
  
  // 初始化引脚
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // 创建WiFi热点
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("📡 WiFi热点已创建");
  Serial.print("📶 热点名称: ");
  Serial.println(ap_ssid);
  Serial.print("🔑 密码: ");
  Serial.println(ap_password);
  Serial.print("🌐 IP地址: ");
  Serial.println(WiFi.softAPIP());

  // 设置Web服务器路由
  server.on("/", handleRoot);
  server.on("/api/status", handleApiStatus);
  server.on("/api/arm", HTTP_POST, handleApiArm);
  server.on("/api/disarm", HTTP_POST, handleApiDisarm);
  
  // 启动服务器
  server.begin();
  Serial.println("✅ Web服务器已启动");
  Serial.println("请在手机连接热点后访问: http://192.168.4.1");
}

// ========== 主循环 ==========
void loop() {
  // 处理Web请求
  server.handleClient();
  
  // 读取触摸值
  touchValue = touchRead(TOUCH_PIN);
  isTouched = (touchValue < TOUCH_THRESHOLD);
  
  // ===== 状态机逻辑 =====
  switch(currentState) {
    case STATE_DISARMED:
      // 撤防状态: LED关闭，忽略触摸
      if (ledState) {
        ledState = false;
        digitalWrite(LED_PIN, LOW);
      }
      break;
      
    case STATE_ARMED:
      // 布防状态: 检测触摸
      if (isTouched) {
        // 触发报警
        currentState = STATE_ALARM;
        alarmStartTime = millis();
        lastLedToggle = 0;
        Serial.println("🚨 报警触发!!!");
      }
      break;
      
    case STATE_ALARM:
      // 报警状态: LED高频闪烁
      unsigned long currentTime = millis();
      if (currentTime - lastLedToggle >= ALARM_BLINK_INTERVAL) {
        lastLedToggle = currentTime;
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
      }
      break;
  }
  
  // 调试输出 (每5秒打印一次)
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 5000) {
    lastPrint = millis();
    Serial.print("触摸值: ");
    Serial.print(touchValue);
    Serial.print(" | 触摸: ");
    Serial.print(isTouched ? "YES" : "NO");
    Serial.print(" | 状态: ");
    switch(currentState) {
      case STATE_DISARMED: Serial.println("撤防"); break;
      case STATE_ARMED: Serial.println("布防"); break;
      case STATE_ALARM: Serial.println("报警!!!"); break;
    }
  }
  
  delay(50); // 防止看门狗复位
}
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_dai";
const char* password = "12345678";

const int touchPin = 4;
WebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);

  server.begin();
  Serial.println("传感器仪表盘已启动");
}

void loop() {
  server.handleClient();
}

// ===== 网页 =====
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8" name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 传感器仪表盘</title>
    <style>
        body { font-family: Arial; text-align: center; padding: 20px; background: #0d1117; color: #c9d1d9; }
        h1 { color: #58a6ff; }
        .container { max-width: 500px; margin: 0 auto; }
        .value-box {
            background: #161b22;
            border: 2px solid #30363d;
            border-radius: 16px;
            padding: 40px 20px;
            margin: 20px 0;
        }
        #sensorValue {
            font-size: 80px;
            font-weight: bold;
            color: #58a6ff;
            transition: color 0.3s;
        }
        .label { font-size: 18px; color: #8b949e; }
        .status-bar {
            display: flex;
            justify-content: space-between;
            background: #161b22;
            border-radius: 10px;
            padding: 12px 20px;
            margin: 10px 0;
            font-size: 14px;
            color: #8b949e;
        }
        .status-bar span { color: #58a6ff; font-weight: bold; }
        .progress-bar {
            width: 100%;
            height: 20px;
            background: #30363d;
            border-radius: 10px;
            overflow: hidden;
            margin: 15px 0;
        }
        .progress-fill {
            height: 100%;
            width: 0%;
            background: linear-gradient(to right, #58a6ff, #1f6feb);
            border-radius: 10px;
            transition: width 0.2s;
        }
        .tip { font-size: 14px; color: #8b949e; margin-top: 20px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>📊 传感器仪表盘</h1>
        <div class="value-box">
            <div class="label">触摸传感器实时数值</div>
            <div id="sensorValue">--</div>
        </div>
        <div class="progress-bar">
            <div class="progress-fill" id="progressFill"></div>
        </div>
        <div class="status-bar">
            <span>📡 状态: <span id="statusText">正在读取...</span></span>
            <span>🔄 更新: <span id="updateCount">0</span> 次</span>
        </div>
        <p class="tip">✋ 用手靠近 GPIO4 引脚，数值会实时变小</p >
    </div>

    <script>
        let count = 0;
        const MAX_VALUE = 100;  // 触摸读数通常 < 100

        function fetchData() {
            fetch('/data')
                .then(response => response.text())
                .then(value => {
                    const num = parseInt(value);
                    if (!isNaN(num)) {
                        document.getElementById('sensorValue').textContent = num;
                        // 更新进度条
                        let percent = Math.min((num / 100) * 100, 100);
                        document.getElementById('progressFill').style.width = percent + '%';
                        // 状态
                        if (num < 40) {
                            document.getElementById('sensorValue').style.color = '#f0883e';
                            document.getElementById('statusText').textContent = '✋ 触摸检测到';
                        } else {
                            document.getElementById('sensorValue').style.color = '#58a6ff';
                            document.getElementById('statusText').textContent = '✅ 空闲';
                        }
                        document.getElementById('updateCount').textContent = ++count;
                    }
                })
                .catch(err => {
                    document.getElementById('statusText').textContent = '❌ 连接断开';
                });
        }

        // 每 200ms 刷新一次数据
        setInterval(fetchData, 200);
        fetchData();
    </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

// ===== 返回触摸传感器原始数值 =====
void handleData() {
  int val = touchRead(touchPin);
  server.send(200, "text/plain", String(val));
}
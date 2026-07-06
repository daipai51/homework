#include <WiFi.h>
#include <WebServer.h>

const char* ap_ssid = "ESP32-dai";
const char* ap_pass = "12345678";

// 三路LED引脚 2、4、5
const int LED1 = 2;
const int LED2 = 4;
const int LED3 = 5;

int bri1 = 0, bri2 = 0, bri3 = 0;
WebServer server(80);

String makeHtml()
{
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>三路顺滑调光</title>
<style>
body{background:#111;color:#fff;font-family:Arial;text-align:center;padding-top:50px;}
.box{max-width:520px;margin:0 auto;padding:30px 20px;background:#222;border-radius:16px;}
.line{margin:24px 0;}
label{font-size:19px;display:block;margin-bottom:10px;}
.val{font-size:22px;color:#ffd840;margin:6px 0;}
input[type=range]{width:100%;height:12px;}
</style>
</head>
<body>
<div class="box">
<h1>三路LED独立调光</h1>

<div class="line">
  <label>LED1 GPIO2</label>
  <div class="val">亮度：<span id="v1">0</span></div>
  <input type="range" id="s1" min="0" max="255" value="0">
</div>

<div class="line">
  <label>LED2 GPIO4</label>
  <div class="val">亮度：<span id="v2">0</span></div>
  <input type="range" id="s2" min="0" max="255" value="0">
</div>

<div class="line">
  <label>LED3 GPIO5</label>
  <div class="val">亮度：<span id="v3">0</span></div>
  <input type="range" id="s3" min="0" max="255" value="0">
</div>
</div>

<script>
const s1 = document.getElementById("s1");
const s2 = document.getElementById("s2");
const s3 = document.getElementById("s3");
const v1 = document.getElementById("v1");
const v2 = document.getElementById("v2");
const v3 = document.getElementById("v3");

// 节流防抖变量：限制100ms仅发送一次请求
let timer = null;
function sendLight(){
  // 清除上一次延时，避免重复发送
  clearTimeout(timer);
  timer = setTimeout(()=>{
    let a = s1.value, b = s2.value, c = s3.value;
    v1.innerText = a;
    v2.innerText = b;
    v3.innerText = c;
    // 异步发送亮度
    fetch(`/set?b1=${a}&b2=${b}&b3=${c}`);
  },100);
}

// 所有滑块共用同一个节流函数
s1.oninput = sendLight;
s2.oninput = sendLight;
s3.oninput = sendLight;
</script>
</body>
</html>
)rawliteral";
  return html;
}

void handleRoot()
{
  server.send(200, "text/html;charset=UTF-8", makeHtml());
}

void handleSet()
{
  // 读取亮度参数
  if(server.hasArg("b1")) bri1 = server.arg("b1").toInt();
  if(server.hasArg("b2")) bri2 = server.arg("b2").toInt();
  if(server.hasArg("b3")) bri3 = server.arg("b3").toInt();

  bri1 = constrain(bri1,0,255);
  bri2 = constrain(bri2,0,255);
  bri3 = constrain(bri3,0,255);

  analogWrite(LED1, bri1);
  analogWrite(LED2, bri2);
  analogWrite(LED3, bri3);

  // 快速返回，不要加串口打印拖慢响应
  server.send(200, "text/plain", "");
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  analogWrite(LED1,0);
  analogWrite(LED2,0);
  analogWrite(LED3,0);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);

  Serial.print("热点：");Serial.println(ap_ssid);
  Serial.print("密码：");Serial.println(ap_pass);
  Serial.print("IP：");Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();
}

void loop()
{
  server.handleClient();
}
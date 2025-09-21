#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
#include "claw_control.h"

// WiFi配置
const char* ssid = "2509";
const char* password = "250925092509";

// 创建Web服务器实例
ESP8266WebServer server(80);

// 自动模式状态变量
bool autoModeRunning = false;
unsigned long lastAutoModeChange = 0;
const int AUTO_MODE_DELAY = 2000; // 2秒自动模式间隔

// 网页HTML内容
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>机械爪控制</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 20px;
            background-color: #f0f0f0;
        }
        .container {
            max-width: 500px;
            margin: 0 auto;
            background-color: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
        }
        .btn {
            display: inline-block;
            width: 150px;
            height: 50px;
            margin: 10px;
            font-size: 18px;
            font-weight: bold;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            transition: all 0.3s;
        }
        .open-btn {
            background-color: #4CAF50;
            color: white;
        }
        .close-btn {
            background-color: #f44336;
            color: white;
        }
        .stop-btn {
            background-color: #ff9800;
            color: white;
        }
        .auto-btn {
            background-color: #2196F3;
            color: white;
        }
        .btn:hover {
            opacity: 0.8;
            transform: scale(1.05);
        }
        .status {
            font-size: 20px;
            margin: 20px 0;
            padding: 10px;
            border-radius: 5px;
            background-color: #e0e0e0;
        }
        .control-panel {
            margin: 20px 0;
        }
        .slider-container {
            margin: 20px 0;
        }
        .slider {
            width: 80%;
            height: 20px;
            margin: 10px auto;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP8266机械爪控制</h1>
        <div class="status" id="status">状态: 等待连接...</div>

        <div class="control-panel">
            <button class="btn open-btn" onclick="openClaw()">张开爪子</button>
            <button class="btn close-btn" onclick="closeClaw()">闭合爪子</button>
            <br>
            <button class="btn stop-btn" onclick="stopClaw()">停止</button>
            <button class="btn auto-btn" onclick="autoMode()">自动模式</button>
        </div>

        <div class="slider-container">
            <label for="positionSlider">位置控制:</label>
            <input type="range" min="0" max="360" value="90" class="slider" id="positionSlider" onchange="setPosition(this.value)">
            <p>当前角度: <span id="angleValue">90</span>°</p>
        </div>
    </div>

    <script>
        // 更新状态显示
        function updateStatus(message) {
            document.getElementById("status").innerHTML = "状态: " + message;
        }

        // 张开爪子
        function openClaw() {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/open", true);
            xhr.onreadystatechange = function() {
                if (xhr.readyState === 4 && xhr.status === 200) {
                    updateStatus("爪子张开中...");
                }
            };
            xhr.send();
        }

        // 闭合爪子
        function closeClaw() {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/close", true);
            xhr.onreadystatechange = function() {
                if (xhr.readyState === 4 && xhr.status === 200) {
                    updateStatus("爪子闭合中...");
                }
            };
            xhr.send();
        }

        // 停止运动
        function stopClaw() {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/stop", true);
            xhr.onreadystatechange = function() {
                if (xhr.readyState === 4 && xhr.status === 200) {
                    updateStatus("运动已停止");
                }
            };
            xhr.send();
        }

        // 自动模式
        function autoMode() {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/auto", true);
            xhr.onreadystatechange = function() {
                if (xhr.readyState === 4 && xhr.status === 200) {
                    updateStatus("自动模式运行中...");
                }
            };
            xhr.send();
        }

        // 设置位置
        function setPosition(angle) {
            document.getElementById("angleValue").innerHTML = angle;
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/setPosition?angle=" + angle, true);
            xhr.onreadystatechange = function() {
                if (xhr.readyState === 4 && xhr.status === 200) {
                    updateStatus("设置角度: " + angle + "°");
                }
            };
            xhr.send();
        }

        // 页面加载完成后更新状态
        window.onload = function() {
            updateStatus("已连接到机械爪控制器");
        }
    </script>
</body>
</html>
)=====";

// 发送主页面
void handleRoot() {
  server.send(200, "text/html", MAIN_page);
}

// 张开爪子
void handleOpen() {
  openClaw();
  server.send(200, "text/plain", "Claw Opening");
}

// 闭合爪子
void handleClose() {
  closeClaw();
  server.send(200, "text/plain", "Claw Closing");
}

// 停止运动
void handleStop() {
  stopClaw();
  autoModeRunning = false;
  server.send(200, "text/plain", "Claw Stopped");
}

// 设置指定角度
void handleSetPosition() {
  if (server.hasArg("angle")) {
    int angle = server.arg("angle").toInt();
    setPosition(angle);
    server.send(200, "text/plain", "Position Set to " + String(angle));
  } else {
    server.send(400, "text/plain", "Missing angle parameter");
  }
}

// 自动模式 - 切换自动模式状态
void handleAuto() {
  autoModeRunning = !autoModeRunning;
  lastAutoModeChange = millis();
  String message = autoModeRunning ? "Auto Mode Started" : "Auto Mode Stopped";
  server.send(200, "text/plain", message);
}

// 404页面
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  Serial.println();

  // 连接WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to WiFi, IP address: ");
  Serial.println(WiFi.localIP());

  // 初始化舵机
  initClaw();

  // 配置路由
  server.on("/", handleRoot);
  server.on("/open", handleOpen);
  server.on("/close", handleClose);
  server.on("/stop", handleStop);
  server.on("/setPosition", handleSetPosition);
  server.on("/auto", handleAuto);
  server.onNotFound(handleNotFound);

  // 启动服务器
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // 处理HTTP请求
  server.handleClient();

  // 自动模式控制逻辑
  if (autoModeRunning) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastAutoModeChange >= AUTO_MODE_DELAY) {
      // 切换爪子状态
      if (getClawState() == CLAW_OPEN) {
        closeClaw();
      } else {
        openClaw();
      }
      lastAutoModeChange = currentMillis;
    }
  }
}
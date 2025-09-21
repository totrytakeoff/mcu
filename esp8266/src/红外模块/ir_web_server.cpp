#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// WiFi配置
const char* ssid = "2509";
const char* password = "250925092509";

// 红外接收配置
const uint16_t kRecvPin = 14; // D5引脚 on NodeMCU
IRrecv irrecv(kRecvPin);
decode_results results;

// Web服务器和WebSocket服务器
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// 按键映射表
struct IrKey {
  unsigned long value;
  String name;
};

IrKey irKeys[] = {
  {0xFFA25D, "KEY_1"},
  {0xFF629D, "KEY_2"},
  {0xFFE21D, "KEY_3"},
  {0xFF22DD, "KEY_4"},
  {0xFF02FD, "KEY_5"},
  {0xFFC23D, "KEY_6"},
  {0xFFE01F, "KEY_7"},
  {0xFFA857, "KEY_8"},
  {0xFF906F, "KEY_9"},
  {0xFF9867, "KEY_0"},
  {0xFF6897, "KEY_STAR"},
  {0xFFB04F, "KEY_HASH"},
  {0xFF18E7, "KEY_UP"},
  {0xFF4AB5, "KEY_DOWN"},
  {0xFF10EF, "KEY_LEFT"},
  {0xFF5AA5, "KEY_RIGHT"},
  {0xFF38C7, "KEY_OK"}
};

const int keyCount = sizeof(irKeys) / sizeof(IrKey);

// 获取按键名称
String getKeyName(unsigned long value) {
  for (int i = 0; i < keyCount; i++) {
    if (irKeys[i].value == value) {
      return irKeys[i].name;
    }
  }
  return "UNKNOWN";
}

// 处理根路径请求
void handleRoot() {
  String html = "<!DOCTYPE html>\n";
  html += "<html>\n";
  html += "<head>\n";
  html += "  <title>IR Remote Control</title>\n";
  html += "  <meta charset='utf-8'>\n";
  html += "  <meta name='viewport' content='width=device-width, initial-scale=1'>\n";
  html += "  <style>\n";
  html += "    body { font-family: Arial, sans-serif; text-align: center; margin: 20px; }\n";
  html += "    #keyDisplay { font-size: 24px; margin: 20px; padding: 20px; border: 2px solid #333; }\n";
  html += "    .key { display: inline-block; width: 80px; height: 40px; margin: 5px; padding: 10px; background-color: #eee; border: 1px solid #999; cursor: pointer; }\n";
  html += "    .pressed { background-color: #4CAF50; color: white; }\n";
  html += "  </style>\n";
  html += "</head>\n";
  html += "<body>\n";
  html += "  <h1>IR Remote Control</h1>\n";
  html += "  <div id='keyDisplay'>Press a key on your remote</div>\n";
  html += "  <div id='remote'>\n";

  // 创建遥控器界面
  for (int i = 0; i < keyCount; i++) {
    html += "    <div class='key' id='" + irKeys[i].name + "'>" + irKeys[i].name + "</div>\n";
  }

  html += "  </div>\n";
  html += "  <script>\n";
  html += "    var connection = new WebSocket('ws://' + window.location.hostname + ':81');\n";
  html += "    connection.onmessage = function(event) {\n";
  html += "      var data = JSON.parse(event.data);\n";
  html += "      document.getElementById('keyDisplay').innerText = 'Pressed: ' + data.key + ' (0x' + data.value + ')';\n";
  html += "      \n";
  html += "      // 高亮显示按下的按键\n";
  html += "      var keys = document.getElementsByClassName('key');\n";
  html += "      for (var i = 0; i < keys.length; i++) {\n";
  html += "        keys[i].classList.remove('pressed');\n";
  html += "      }\n";
  html += "      var pressedKey = document.getElementById(data.key);\n";
  html += "      if (pressedKey) {\n";
  html += "        pressedKey.classList.add('pressed');\n";
  html += "      }\n";
  html += "    };\n";
  html += "  </script>\n";
  html += "</body>\n";
  html += "</html>\n";

  server.send(200, "text/html", html);
}

// WebSocket事件处理
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      break;
  }
}

// 发送红外按键信息到所有连接的客户端
void sendKeyToClients(unsigned long value, String keyName) {
  String jsonData = "{\"value\":\"" + String(value, HEX) + "\",\"key\":\"" + keyName + "\"}";
  webSocket.broadcastTXT(jsonData);
}

void setup() {
  // 启动串口通信
  Serial.begin(115200);
  Serial.println("IR Web Server starting...");

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

  // 初始化红外接收器
  irrecv.enableIRIn();

  // 配置Web服务器路由
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

  // 启动WebSocket服务器
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started");
}

void loop() {
  // 处理Web服务器请求
  server.handleClient();

  // 处理WebSocket事件
  webSocket.loop();

  // 检查是否接收到红外信号
  if (irrecv.decode(&results)) {
    unsigned long value = results.value;

    // 处理重复按键和长按按键
    static unsigned long lastValue = 0;
    static unsigned long lastTime = 0;
    unsigned long currentTime = millis();

    // 0xFFFFFFFF 是长按按键产生的重复码，忽略它
    // 只有当按键值不同或者是间隔足够长的重复按键时才处理
    if (value != 0xFFFFFFFF || (currentTime - lastTime) > 200) {
      String keyName = getKeyName(value);

      // 如果是0xFFFFFFFF，使用上一次的按键名称
      if (value == 0xFFFFFFFF && lastValue != 0) {
        keyName = getKeyName(lastValue) + "_HOLD";
      }

      // 打印接收到的红外代码
      Serial.print("Received IR code: 0x");
      Serial.print(value, HEX);
      Serial.print(" (");
      Serial.print(keyName);
      Serial.println(")");

      // 发送到WebSocket客户端
      sendKeyToClients(value, keyName);

      // 更新最后按键值和时间
      if (value != 0xFFFFFFFF) {
        lastValue = value;
      }
      lastTime = currentTime;
    }

    // 继续接收下一个值
    irrecv.resume();
  }

  delay(100);
}
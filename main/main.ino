// 不要用VSCode烧写，一定要用Arduino IDE烧写
#include <FS.h>  
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>  
#include <ESP8266WebServer.h>

Ticker ticker;
ESP8266WebServer esp8266_server(80);    // 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）

void setup() {
  Serial.begin(9600);          // 启动串口通讯
  pinMode(LED_BUILTIN, OUTPUT); //设置内置LED引脚为输出模式以便控制LED
  ticker.attach(1, testLed);  // 设置Ticker对象

  // 自动连接WiFi。以下语句的参数是连接ESP8266时的WiFi名称
  Serial.println("尝试连接WIFI");
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  // WiFi连接成功后将通过串口监视器输出连接成功信息 
  Serial.print("连接到WIFI：");
  Serial.println(WiFi.SSID());              // 通过串口监视器输出连接的WiFi名称
  Serial.print("你的IP为：\t");
  Serial.println(WiFi.localIP());           // 通过串口监视器输出ESP8266-NodeMCU的IP

  // Web服务器初始化
  if(!SPIFFS.begin()){                       // 启动闪存文件系统
    Serial.println("无法启动闪存文件系统，终止程序");
    return;
  } 
  esp8266_server.onNotFound(handleUserRequet);      // 告知系统如何处理用户请求
  esp8266_server.begin();                           // 启动网站服务
  Serial.println("Web服务器已启动");

  // OTA设置并启动
  ArduinoOTA.setHostname("ESP8266");
  ArduinoOTA.setPassword("12345678");
  ArduinoOTA.begin();

}

void loop(void) {
  esp8266_server.handleClient();                    // 处理用户WEB请求
  ArduinoOTA.handle();                              // 处理OTA升级
}

void testLed() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

// 处理用户浏览器的HTTP访问
void handleUserRequet() {

  // server_on
  esp8266_server.on("/led", HTTP_POST, []() {
    Serial.println("server_on");
    digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));// 改变LED的点亮或者熄灭状态
    esp8266_server.sendHeader("Location","/");          // 跳转回页面根目录
  esp8266_server.send(303); 
  });

  // 获取用户请求网址信息
  String webAddress = esp8266_server.uri();
  // 通过handleFileRead函数处处理用户访问
  bool fileReadOK = handleFileRead(webAddress);

  // 如果在SPIFFS无法找到用户访问的资源，则回复404 (Not Found)
  if (!fileReadOK){                                                 
    esp8266_server.send(404, "text/plain", "404该页面不存在"); 
  }
}

bool handleFileRead(String path) {            //处理浏览器HTTP访问

  if (path.endsWith("/")) {                   // 如果访问地址以"/"为结尾
    path = "/index.html";                     // 则将访问地址修改为/index.html便于SPIFFS访问
  } 
  
  String contentType = getContentType(path);  // 获取文件类型
  
  if (SPIFFS.exists(path)) {                     // 如果访问的文件可以在SPIFFS中找到
    File file = SPIFFS.open(path, "r");          // 则尝试打开该文件
    esp8266_server.streamFile(file, contentType);// 并且将该文件返回给浏览器
    file.close();                                // 并且关闭文件
    return true;                                 // 返回true
  }
  return false;                                  // 如果文件未找到，则返回false
}

// 获取文件类型
String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

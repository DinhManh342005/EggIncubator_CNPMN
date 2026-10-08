#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include "web_page.h"
#include <DNSServer.h>         // <--- THÊM THƯ VIỆN NÀY
#include <WiFiManager.h>        // <--- THÊM THƯ VIỆN NÀY (Để config WiFi)
#include <ESP8266mDNS.h>        // <--- THÊM THƯ VIỆN NÀY (Để dùng mayaptrung.local)


// Cấu hình chân SoftwareSerial trên NodeMCU ESP8266
// D5 (GPIO14) làm RX: nối vào chân A1 (TX) của Arduino
// D6 (GPIO12) làm TX: nối vào chân A0 (RX) của Arduino
#define ARDUINO_RX_PIN D5
#define ARDUINO_TX_PIN D6

SoftwareSerial linkSerial(ARDUINO_RX_PIN, ARDUINO_TX_PIN);

// const char* ssid = "Trung Tam TT-TV T3";
// const char* password = "12345679";
// const char* ssid = "Dung-4G_di";
// const char* password = "56320282";

const char* www_username = "manhdinh";
const char* www_password = "123456";

ESP8266WebServer server(80);

float temp = 0.0;
int humi = 0, day = 1, light = 0, fan = 0;
int profileIdx = 0;
int stage = 0; // 0: Ấp, 1: Nở
float targetTemp = 37.5;
int targetHumi = 60;

bool kiemTraDangNhap() {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return false;
  }
  return true;
}

void handleRoot() {
  if (!kiemTraDangNhap()) return;
  server.send(200, "text/html", INDEX_HTML);
}

void handleData() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  // Tạo bộ đệm cố định, giải phóng RAM ngay sau khi gửi xong
  char jsonBuf[256];
  snprintf(jsonBuf, sizeof(jsonBuf),
    "{\"temp\":%.1f,\"humi\":%d,\"day\":%d,\"light\":%d,\"fan\":%d,\"profile\":%d,\"stage\":%d,\"targetTemp\":%.1f,\"targetHumi\":%d}",
    temp, humi, day, light, fan, profileIdx, stage, targetTemp, targetHumi);

  server.send(200, "application/json", jsonBuf);
}

void setupRoutes() {
  server.on("/", handleRoot);
  server.on("/api/data", handleData);

  server.on("/api/toggle", []() {
    if (!kiemTraDangNhap()) return;
    String dev = server.arg("device");
    int val = server.arg("val").toInt();
    if (dev == "heater") {
      light = val;
      linkSerial.println("CMD:HEATER:" + String(val));
      Serial.println("[ESP GUI ARDUINO]: CMD:HEATER:" + String(val));
    } else if (dev == "fan") {
      fan = val;
      linkSerial.println("CMD:FAN:" + String(val));
      Serial.println("[ESP GUI ARDUINO]: CMD:FAN:" + String(val));
    }
    server.send(200, "text/plain", "OK");
  });

  server.on("/api/profile", []() {
    if (!kiemTraDangNhap()) return;
    String val = server.arg("val");
    profileIdx = val.toInt();
    linkSerial.println("CMD:PROFILE:" + val);
    Serial.println("[ESP GUI ARDUINO]: CMD:PROFILE:" + val);
    server.send(200, "text/plain", "OK");
  });

  server.on("/api/set_day", []() {
    if (!kiemTraDangNhap()) return;
    String val = server.arg("val");
    linkSerial.println("CMD:DAY:" + val);
    Serial.println("[ESP GUI ARDUINO]: CMD:DAY:" + val);
    server.send(200, "text/plain", "OK");
  });

  server.on("/api/custom_profile", []() {
    if (!kiemTraDangNhap()) return;
    String cmd = "CMD:SET:" + server.arg("tAp") + "," + server.arg("hAp") + "," +
                 server.arg("tNo") + "," + server.arg("hNo") + "," +
                 server.arg("days") + "," + server.arg("hatch");
    linkSerial.println(cmd);
    Serial.println("[ESP GUI ARDUINO]: " + cmd);
    server.send(200, "text/plain", "OK");
  });

  server.on("/api/run", []() {
    if (!kiemTraDangNhap()) return;
    String val = server.arg("val");
    linkSerial.println("CMD:RUN:" + val);
    Serial.println("[ESP GUI ARDUINO]: CMD:RUN:" + val);
    server.send(200, "text/plain", "OK");
  });
}

// Đọc và xử lý gói tin Serial gửi từ Arduino Uno
void docDuLieuArduino() {
  static String rxBuffer = ""; // Bộ đệm lưu chuỗi nhận được

  while (linkSerial.available()) {
    char c = (char)linkSerial.read();

    if (c == '\n') { // Gặp ký tự kết thúc gói tin
      rxBuffer.trim();

      if (rxBuffer.length() > 0) {
        float _t, _tt;
        int _h, _d, _l, _f, _p, _s, _th;

        // Trích xuất các thông số từ chuỗi gói tin
        int parsed = sscanf(rxBuffer.c_str(), "T:%f,H:%d,Day:%d,L:%d,F:%d,P:%d,S:%d,TT:%f,TH:%d", 
                            &_t, &_h, &_d, &_l, &_f, &_p, &_s, &_tt, &_th);

        if (parsed >= 5) { // Đọc thành công các thông số cơ bản
          temp = _t; humi = _h; day = _d; light = _l; fan = _f;

          if (parsed >= 9) { // Đọc thành công đầy đủ thông số cài đặt
            profileIdx = _p; stage = _s; targetTemp = _tt; targetHumi = _th;
          }
        }
      }
      rxBuffer = ""; // Reset bộ đệm sau khi xử lý xong
    } 
    else if (c != '\r') {
      rxBuffer += c;
      if (rxBuffer.length() > 120) rxBuffer = ""; // Tránh tràn RAM khi nhiễu tín hiệu
    }
  }
}

 void setup() {
  Serial.begin(115200);          // Serial cứng để cắm cáp debug máy tính
  linkSerial.begin(38400);        // SoftwareSerial nối sang chân A0/A1 của Arduino Uno           
  
  // ================= CẤU HÌNH WIFIMANAGER =================
  WiFiManager wifiManager;

  // Tên máy được giữ ổn định trên mạng LAN và dùng cho mDNS.
  wifiManager.setHostname("mayaptrung");

  // Nếu không kết nối được WiFi cũ trong 180 giây (3 phút),
  // ESP sẽ giữ nguyên AP "eggincubator_config" để bạn cấu hình lại.
  wifiManager.setConfigPortalTimeout(180);

  // Phát WiFi tên "eggincubator_config" (mật khẩu mặc định: 12345678)
  // Khi điện thoại bắt WiFi này, trang cấu hình chọn WiFi/Nhập mật khẩu sẽ TỰ ĐỘNG BẬT LÊN!
  if (!wifiManager.autoConnect("EggIncubator_Config", "12345678")) {
    Serial.println("Khong ket noi duoc WiFi va het thoi gian chờ Config! Khoi dong lai...");
    delay(3000);
    ESP.restart(); // Khởi động lại để thử kết nối lại
  }

  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  Serial.println("\n-> WiFi ket noi thanh cong!");
  Serial.print("-> IP Web: ");
  Serial.println(WiFi.localIP());
  Serial.print("-> Hostname: ");
  Serial.println(WiFi.hostname());

  // ================= CẤU HÌNH TEN MIEN .LOCAL (mDNS) =================
  if (MDNS.begin("mayaptrung")) { // Tên miền: http://mayaptrung.local
    Serial.println("-> mDNS responder da khoi tao! Mien truy cap: http://mayaptrung.local");
  } else {
    Serial.println("-> Loi khoi tao mDNS!");
  }

  // Khởi tạo các đường dẫn Web Server
  setupRoutes();

  // Bổ sung dịch vụ HTTP cho mDNS
  MDNS.addService("http", "tcp", 80);
  server.begin();
}


void loop() {
  MDNS.update();        // <--- THÊM DÒNG NÀY để duy trì tên miền mayaptrung.local
  server.handleClient();
  docDuLieuArduino();

 // Dọn dẹp rác RAM nếu bộ nhớ xuống dưới 6KB
  if (ESP.getFreeHeap() < 6000) {
    delay(10);
  }
}
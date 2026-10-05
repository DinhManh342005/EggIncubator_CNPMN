#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include "web_page.h"

// Cấu hình chân SoftwareSerial trên NodeMCU ESP8266
// D5 (GPIO14) làm RX: nối vào chân A1 (TX) của Arduino
// D6 (GPIO12) làm TX: nối vào chân A0 (RX) của Arduino
#define ARDUINO_RX_PIN D5
#define ARDUINO_TX_PIN D6

SoftwareSerial linkSerial(ARDUINO_RX_PIN, ARDUINO_TX_PIN);

const char* ssid = "Trung Tam TT-TV T3";
const char* password = "12345679";
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
  String json = "{\"temp\":" + String(temp, 1) + 
                ",\"humi\":" + String(humi) + 
                ",\"day\":" + String(day) + 
                ",\"light\":" + String(light) + 
                ",\"fan\":" + String(fan) + 
                ",\"profile\":" + String(profileIdx) + 
                ",\"stage\":" + String(stage) + 
                ",\"targetTemp\":" + String(targetTemp, 1) + 
                ",\"targetHumi\":" + String(targetHumi) + "}";
  server.send(200, "application/json", json);
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

// Đọc gói tin gửi từ Arduino qua SoftwareSerial (D5/D6)
void docDuLieuArduino() {
  static String rxBuffer = "";
  
  while (linkSerial.available()) {
    char c = (char)linkSerial.read();
    if (c == '\n') {
      rxBuffer.trim();
      if (rxBuffer.length() > 0) {
        Serial.println("[ESP NHAN DUOC]: " + rxBuffer);

        int idxT  = rxBuffer.indexOf("T:");
        int idxH  = rxBuffer.indexOf(",H:");
        int idxD  = rxBuffer.indexOf(",Day:");
        int idxL  = rxBuffer.indexOf(",L:");
        int idxF  = rxBuffer.indexOf(",F:");
        int idxP  = rxBuffer.indexOf(",P:");
        int idxS  = rxBuffer.indexOf(",S:");
        int idxTT = rxBuffer.indexOf(",TT:");
        int idxTH = rxBuffer.indexOf(",TH:");

        if (idxT >= 0 && idxH > idxT && idxD > idxH && idxL > idxD && idxF > idxL) {
          temp  = rxBuffer.substring(idxT + 2, idxH).toFloat();
          humi  = rxBuffer.substring(idxH + 3, idxD).toInt();
          day   = rxBuffer.substring(idxD + 5, idxL).toInt();
          light = rxBuffer.substring(idxL + 3, idxF).toInt();

          if (idxP > idxF && idxS > idxP && idxTT > idxS && idxTH > idxTT) {
            fan        = rxBuffer.substring(idxF + 3, idxP).toInt();
            profileIdx = rxBuffer.substring(idxP + 3, idxS).toInt();
            stage      = rxBuffer.substring(idxS + 3, idxTT).toInt();
            targetTemp = rxBuffer.substring(idxTT + 4, idxTH).toFloat();
            targetHumi = rxBuffer.substring(idxTH + 4).toInt();
          } else {
            fan = rxBuffer.substring(idxF + 3).toInt();
          }
          Serial.printf("[ESP PARSE OK]: T=%.1f, H=%d, Day=%d, L=%d, F=%d, P=%d, S=%d, TT=%.1f, TH=%d\n",
                        temp, humi, day, light, fan, profileIdx, stage, targetTemp, targetHumi);
        } else {
          Serial.println("[ESP PARSE LOI]: Sai cu phap goi tin!");
        }
      }
      rxBuffer = "";
    } else if (c != '\r') {
      rxBuffer += c;
      if (rxBuffer.length() > 120) rxBuffer = ""; // Reset chong tran bo dem khi co nhieu
    }
  }
}

void setup() {
  Serial.begin(115200);          // Serial cứng để cắm cáp debug máy tính
  linkSerial.begin(9600);        // SoftwareSerial nối sang chân A0/A1 của Arduino Uno
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Dang ket noi WiFi ");
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi ket noi thanh cong! IP Web: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nKhong the ket noi WiFi! Tu dong phat Access Point AP: MayApTrung_WiFi (Pass: 12345678)");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("MayApTrung_WiFi", "12345678");
    Serial.println("AP IP Web: " + WiFi.softAPIP().toString());
  }

  setupRoutes();
  server.begin();
}

void loop() {
  server.handleClient();
  docDuLieuArduino();
}
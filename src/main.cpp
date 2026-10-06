#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <RtcDS1302.h>
#include <SoftwareSerial.h>

// ================= CẤU HÌNH CHÂN PHẦN CỨNG =================
#define DHTPIN 2
#define DHTTYPE DHT11

#define RELAY_FAN 4          // Relay Quạt: D4
#define RELAY_HEATER 5       // Relay Đèn sưởi: D5

// Mức kích relay
#define RELAY_ON  HIGH       
#define RELAY_OFF LOW        

#define BTN_1 6              // Nút 1 (D6): EDIT (Mode 1) / OK (Mode 2)
#define BTN_2 7              // Nút 2 (D7): CHỌN (Mode 1) / UP (Mode 2)
#define BTN_3 8              // Nút 3 (D8): MENU (Mode 0) / NEXT (Mode 1) / DOWN (Mode 2)
#define BUZZER_PIN 9         
#define BUZZER_ON  HIGH
#define BUZZER_OFF LOW

#define RTC_CLK 10
#define RTC_RST 11
#define RTC_DAT 12

#define ESP_RX_PIN A0        
#define ESP_TX_PIN A1        

// ================= KHỞI TẠO ĐỐI TƯỢNG =================
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

ThreeWire myWire(RTC_DAT, RTC_CLK, RTC_RST);
RtcDS1302<ThreeWire> Rtc(myWire);

SoftwareSerial espSerial(ESP_RX_PIN, ESP_TX_PIN);

// ================= CẤU TRÚC HỒ SƠ ẤP TRỨNG =================
struct EggProfile {
  char ten[12];
  char maTen[5];
  float nhietAp;       
  int doAmAp;
  float nhietNo;       
  int doAmNo;
  int tongNgay;        
  int ngayBatDauNo;    
};

const EggProfile PROFILES_MAC_DINH[7] = {
  {"TRUNG GA",   "GA",  37.5, 60, 37.0, 70, 21, 19},
  {"TRUNG VIT",  "VIT", 37.8, 65, 37.1, 75, 28, 25},
  {"CHIM CUT",   "CUT", 37.5, 55, 37.0, 65, 17, 15},
  {"BO CAU",     "CAU", 38.0, 58, 37.3, 68, 18, 16},
  {"NGONG",      "NGON",37.7, 68, 37.0, 80, 30, 27},
  {"CHIM TRI",   "TRI", 37.6, 60, 37.0, 70, 24, 21},
  {"CHE DO TEST","TEST",27.0, 60, 28.0, 65, 3, 2}
};

// ================= BIẾN HỆ THỐNG =================
int currentProfileIndex = 0;   
int selectedProfileIndex = 0;  
EggProfile currentProfile;

float targetTemp = 37.5;
int targetHumi = 60;
int tongNgayAp = 21;
int ngayHienTai = 1;

// Biến chỉnh sửa Mode 2
float editTemp = 37.5;
int editHumi = 60;
int editStep = 0; 

int currentMode = 0; 

float nhietDo = 0.0;
float doAm = 0.0;
bool trangThaiDen = true;
bool trangThaiQuat = true;

unsigned long timerDHT = 0;
unsigned long timerLCD = 0;
unsigned long timerGuiESP = 0;

unsigned long lastPress1 = 0;
unsigned long lastPress2 = 0;
unsigned long lastPress3 = 0;
const unsigned long debounceTime = 200;

// ================= QUẢN LÝ CÒI BÁO HIỆU THÔNG MINH =================
enum BuzzerMode {
  BUZZ_OFF = 0,
  BUZZ_CONFIRM_1,     // 1 tiếng bíp ngắn 70ms
  BUZZ_CONFIRM_2,     // 2 tiếng bíp dứt khoát
  BUZZ_OVERHEAT,      // Quá nhiệt nguy hiểm (Bíp dồn dập)
  BUZZ_UNDERHEAT,     // Quá lạnh (5 nhịp, nghỉ 30s)
  BUZZ_HUMI_ALERT     // Cảnh báo ẩm
};

BuzzerMode currentBuzzerMode = BUZZ_OFF;
unsigned long buzzerTimer = 0;
int buzzerStep = 0;

// Biến điều khiển Bíp phản hồi thao tác ưu tiên
volatile unsigned long beepActionEndTime = 0;

unsigned long timerRTC = 0;
uint8_t lastRtcDay = 0;

bool mayDangChay = true;
bool manualOverride = false; 

// Khai báo prototype
void xuLyNutBam();
void capNhatLCD();
void dieuKhienThietBi();
void capNhatGiaiDoanAp();
void guiDuLieuESP();
void docLenhTuESP();
void xuLyCoi();
void kichHoatBipNgan();
void kichHoatBipDoi();
void kiemTraCanhBaoMoiTruong();
void kiemTraTangNgayRTC();

void setBuzzerMode(BuzzerMode mode) {
  if (currentBuzzerMode == mode) return;
  currentBuzzerMode = mode;
  buzzerTimer = millis();
  buzzerStep = 0;
}

// 1 bíp ngắn (70ms) khi bấm nút / thao tác thủ công
void kichHoatBipNgan() {
  beepActionEndTime = millis() + 70;
  digitalWrite(BUZZER_PIN, BUZZER_ON);
}

// 2 bíp dứt khoát khi chọn hồ sơ / lưu cài đặt thành công
void kichHoatBipDoi() {
  currentBuzzerMode = BUZZ_CONFIRM_2;
  buzzerTimer = millis();
  buzzerStep = 0;
}

void kiemTraCanhBaoMoiTruong() {
  if (!mayDangChay) {
    setBuzzerMode(BUZZ_OFF);
    return;
  }

  // 1. Quá nhiệt nguy hiểm: T >= 38.3°C hoặc vượt quá targetTemp 0.8°C
  if (nhietDo >= 38.3 || (targetTemp > 0 && nhietDo >= (targetTemp + 0.8))) {
    setBuzzerMode(BUZZ_OVERHEAT);
  }
  // 2. Quá lạnh (thấp hơn targetTemp 1.5°C)
  else if (targetTemp > 0 && nhietDo > 0 && nhietDo <= (targetTemp - 1.5)) {
    setBuzzerMode(BUZZ_UNDERHEAT);
  }
  // 3. Cảnh báo độ ẩm
  else if ((ngayHienTai >= currentProfile.ngayBatDauNo && doAm > 0 && doAm < 45) || doAm > 88) {
    setBuzzerMode(BUZZ_HUMI_ALERT);
  }
  else {
    setBuzzerMode(BUZZ_OFF);
  }
}

void xuLyCoi() {
  unsigned long now = millis();

  // ƯU TIÊN 1: Âm phản hồi khi bấm nút (không bị chặn bởi bất kỳ cảnh báo nào)
  if (now < beepActionEndTime) {
    digitalWrite(BUZZER_PIN, BUZZER_ON);
    return;
  }

  // ƯU TIÊN 2: Xử lý các chế độ còi cảnh báo môi trường & xác nhận
  switch (currentBuzzerMode) {
    case BUZZ_OFF:
      digitalWrite(BUZZER_PIN, BUZZER_OFF);
      break;

    case BUZZ_CONFIRM_2: {
      // 2 bíp dứt khoát: 80ms ON, 80ms OFF, 80ms ON
      unsigned long elapsed = now - buzzerTimer;
      if (elapsed < 80) {
        digitalWrite(BUZZER_PIN, BUZZER_ON);
      } else if (elapsed < 160) {
        digitalWrite(BUZZER_PIN, BUZZER_OFF);
      } else if (elapsed < 240) {
        digitalWrite(BUZZER_PIN, BUZZER_ON);
      } else {
        digitalWrite(BUZZER_PIN, BUZZER_OFF);
        currentBuzzerMode = BUZZ_OFF;
      }
      break;
    }

    case BUZZ_OVERHEAT: {
      // Dồn dập xả nhiệt: Bíp 120ms ON, 120ms OFF
      unsigned long cycle = (now - buzzerTimer) % 240;
      digitalWrite(BUZZER_PIN, (cycle < 120) ? BUZZER_ON : BUZZER_OFF);
      break;
    }

    case BUZZ_UNDERHEAT: {
      // Báo quá lạnh: 5 tiếng bíp (mỗi tiếng 200ms ON, 800ms OFF), sau đó nghỉ 30s
      unsigned long elapsed = now - buzzerTimer;
      if (buzzerStep < 5) {
        if (elapsed < 200) {
          digitalWrite(BUZZER_PIN, BUZZER_ON);
        } else if (elapsed < 1000) {
          digitalWrite(BUZZER_PIN, BUZZER_OFF);
        } else {
          buzzerStep++;
          buzzerTimer = now;
        }
      } else {
        digitalWrite(BUZZER_PIN, BUZZER_OFF);
        if (elapsed >= 30000UL) {
          buzzerStep = 0;
          buzzerTimer = now;
        }
      }
      break;
    }

    case BUZZ_HUMI_ALERT: {
      // Cảnh báo ẩm: Nhịp đôi 100ms ON, 100ms OFF, 100ms ON, 2000ms OFF
      unsigned long cycle = (now - buzzerTimer) % 2300;
      if (cycle < 100 || (cycle >= 200 && cycle < 300)) {
        digitalWrite(BUZZER_PIN, BUZZER_ON);
      } else {
        digitalWrite(BUZZER_PIN, BUZZER_OFF);
      }
      break;
    }
  }
}

void kiemTraTangNgayRTC() {
  if (Rtc.IsDateTimeValid()) {
    RtcDateTime now = Rtc.GetDateTime();
    if (lastRtcDay == 0) {
      lastRtcDay = now.Day();
    } else if (now.Day() != lastRtcDay) {
      lastRtcDay = now.Day();
      if (mayDangChay && ngayHienTai < tongNgayAp) {
        ngayHienTai++;
        capNhatGiaiDoanAp();
        Serial.print(F("[RTC] Chuyen sang ngay ap moi: "));
        Serial.println(ngayHienTai);
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  espSerial.begin(38400); // Tăng tốc độ từ 9600 -> 38400 Baud
  espSerial.setTimeout(50); 

  Serial.println(F("\n=== HE THONG MAY AP TRUNG KHOI DONG ==="));

  pinMode(RELAY_HEATER, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  digitalWrite(RELAY_HEATER, RELAY_ON);
  digitalWrite(RELAY_FAN, RELAY_ON);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, BUZZER_OFF);

  // Test âm thanh còi 2 tiếng bíp rõ ràng lúc khởi động
  digitalWrite(BUZZER_PIN, BUZZER_ON);
  delay(150);
  digitalWrite(BUZZER_PIN, BUZZER_OFF);
  delay(150);
  digitalWrite(BUZZER_PIN, BUZZER_ON);
  delay(150);
  digitalWrite(BUZZER_PIN, BUZZER_OFF);

  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP);

  dht.begin();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("MAY AP TRUNG");
  lcd.setCursor(0, 1);
  lcd.print("DANG KHOI DONG..");
  delay(800);
  lcd.clear();

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t) && !isnan(h)) {
    nhietDo = t;
    doAm = h;
  }

  Rtc.Begin();
  if (Rtc.GetIsWriteProtected()) Rtc.SetIsWriteProtected(false);
  if (!Rtc.GetIsRunning()) Rtc.SetIsRunning(true);

  if (!Rtc.IsDateTimeValid()) {
    Rtc.SetDateTime(RtcDateTime(__DATE__, __TIME__));
  }

  currentProfile = PROFILES_MAC_DINH[currentProfileIndex];
  capNhatGiaiDoanAp();
  capNhatLCD();
}

void loop() {
  // 0. Đọc lệnh từ ESP
  docLenhTuESP();

  // 1. Quét nút bấm liên tục
  xuLyNutBam();

  // 2. Xử lý âm thanh còi theo nhịp non-blocking
  xuLyCoi();

  // 3. Đọc cảm biến mỗi 2 giây
  if (millis() - timerDHT >= 2000) {
    timerDHT = millis();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      nhietDo = t;
      doAm = h;
    }
    dieuKhienThietBi();
    kiemTraCanhBaoMoiTruong();
  }

  // 4. Kiểm tra RTC mỗi 5 giây
  if (millis() - timerRTC >= 5000) {
    timerRTC = millis();
    kiemTraTangNgayRTC();
  }

  // 5. Cập nhật LCD mỗi 300ms
  if (millis() - timerLCD >= 300) {
    timerLCD = millis();
    capNhatLCD();
  }

  // 6. Gửi dữ liệu sang ESP8266 mỗi 1 giây
  if (millis() - timerGuiESP >= 1000) {
    timerGuiESP = millis();
    guiDuLieuESP();
  }
}

void docLenhTuESP() {
  static String cmdBuffer = "";

  while (espSerial.available()) {
    char c = (char)espSerial.read();

    if (c == '\n') {
      cmdBuffer.trim();

      if (cmdBuffer.length() > 0) {
        Serial.print(F("[UNO NHAN LENH ESP]: "));
        Serial.println(cmdBuffer);

        if (cmdBuffer.startsWith("CMD:HEATER:")) {
          int val = cmdBuffer.substring(11).toInt();
          manualOverride = true;
          trangThaiDen = (val == 1);
          digitalWrite(RELAY_HEATER, trangThaiDen ? RELAY_ON : RELAY_OFF);
          kichHoatBipNgan();
        }
        else if (cmdBuffer.startsWith("CMD:FAN:")) {
          int val = cmdBuffer.substring(8).toInt();
          manualOverride = true;
          trangThaiQuat = (val == 1);
          digitalWrite(RELAY_FAN, trangThaiQuat ? RELAY_ON : RELAY_OFF);
          kichHoatBipNgan();
        }
        else if (cmdBuffer.startsWith("CMD:RUN:")) {
          int val = cmdBuffer.substring(8).toInt();
          mayDangChay = (val == 1);
          manualOverride = false;
          if (!mayDangChay) {
            digitalWrite(RELAY_HEATER, RELAY_OFF);
            digitalWrite(RELAY_FAN, RELAY_OFF);
            trangThaiDen = false;
            trangThaiQuat = false;
          }
          kichHoatBipNgan();
        }
        else if (cmdBuffer.startsWith("CMD:PROFILE:")) {
          int pIdx = cmdBuffer.substring(12).toInt();
          if (pIdx >= 0 && pIdx < 7) {
            currentProfileIndex = pIdx;
            selectedProfileIndex = pIdx;
            currentProfile = PROFILES_MAC_DINH[pIdx];
            manualOverride = false;
            capNhatGiaiDoanAp();
            Serial.print(F("[UNO] Chon ho so thanh cong: "));
            Serial.println(PROFILES_MAC_DINH[pIdx].ten);
            kichHoatBipDoi(); 
          }
        }
        else if (cmdBuffer.startsWith("CMD:DAY:")) {
          int d = cmdBuffer.substring(8).toInt();
          if (d >= 1 && d <= tongNgayAp) {
            ngayHienTai = d;
            capNhatGiaiDoanAp();
            Serial.print(F("[UNO] Chinh ngay ap thanh: "));
            Serial.println(ngayHienTai);
            kichHoatBipDoi(); 
          }
        }
        else if (cmdBuffer.startsWith("CMD:SET:")) {
          String params = cmdBuffer.substring(8);
          int p1 = params.indexOf(',');
          int p2 = params.indexOf(',', p1 + 1);
          int p3 = params.indexOf(',', p2 + 1);
          int p4 = params.indexOf(',', p3 + 1);
          int p5 = params.indexOf(',', p4 + 1);

          if (p1 > 0 && p2 > 0 && p3 > 0 && p4 > 0 && p5 > 0) {
            currentProfile.nhietAp = params.substring(0, p1).toFloat();
            currentProfile.doAmAp = params.substring(p1 + 1, p2).toInt();
            currentProfile.nhietNo = params.substring(p2 + 1, p3).toFloat();
            currentProfile.doAmNo = params.substring(p3 + 1, p4).toInt();
            currentProfile.tongNgay = params.substring(p4 + 1, p5).toInt();
            currentProfile.ngayBatDauNo = params.substring(p5 + 1).toInt();

            manualOverride = false;
            capNhatGiaiDoanAp();
            kichHoatBipDoi(); 
          } 
          else {
            kichHoatBipNgan();
          }
        }
      }
      cmdBuffer = ""; 
    } 
    else if (c != '\r') {
      cmdBuffer += c;
      if (cmdBuffer.length() > 60) cmdBuffer = ""; 
    }
  }
}

void capNhatGiaiDoanAp() {
  tongNgayAp = currentProfile.tongNgay;

  if (ngayHienTai >= currentProfile.ngayBatDauNo) {
    targetTemp = currentProfile.nhietNo;
    targetHumi = currentProfile.doAmNo;
  } else {
    targetTemp = currentProfile.nhietAp;
    targetHumi = currentProfile.doAmAp;
  }
}

void dieuKhienThietBi() {
  if (!mayDangChay) return;

  if (nhietDo <= 0) {
    trangThaiDen = false;
    digitalWrite(RELAY_HEATER, RELAY_OFF);
    manualOverride = false;
    return;
  }

  if (nhietDo >= 38.3 || (targetTemp > 0 && nhietDo >= (targetTemp + 0.8))) {
    trangThaiDen = false;
    trangThaiQuat = true;
    digitalWrite(RELAY_HEATER, RELAY_OFF);
    digitalWrite(RELAY_FAN, RELAY_ON);
    manualOverride = false; 
    return;
  }

  if (manualOverride) return;

  static unsigned long timerQuatNghi = 0;
  static bool dangDaoKhi = false;
  static unsigned long timerDaoKhi = 0;

  if (nhietDo > 0) {
    if (nhietDo < (targetTemp - 0.2)) {
      trangThaiDen = true;
      digitalWrite(RELAY_HEATER, RELAY_ON);
    } 
    else if (nhietDo >= targetTemp) {
      trangThaiDen = false;
      digitalWrite(RELAY_HEATER, RELAY_OFF);
    }

    if (nhietDo >= (targetTemp + 0.4) || (targetHumi > 0 && doAm > (targetHumi + 5))) {
      trangThaiQuat = true;
      dangDaoKhi = false;
      timerQuatNghi = millis();
      digitalWrite(RELAY_FAN, RELAY_ON);
    } 
    else if (nhietDo <= targetTemp && (targetHumi == 0 || doAm <= targetHumi)) {
      if (dangDaoKhi) {
        if (millis() - timerDaoKhi >= 20000UL) {
          dangDaoKhi = false;
          trangThaiQuat = false;
          timerQuatNghi = millis();
          digitalWrite(RELAY_FAN, RELAY_OFF);
        }
      } else {
        if (timerQuatNghi == 0) timerQuatNghi = millis();
        if (millis() - timerQuatNghi >= 900000UL && nhietDo >= 36.0) {
          dangDaoKhi = true;
          timerDaoKhi = millis();
          trangThaiQuat = true;
          digitalWrite(RELAY_FAN, RELAY_ON);
        } else {
          trangThaiQuat = false;
          digitalWrite(RELAY_FAN, RELAY_OFF);
        }
      }
    }
  }
}

void xuLyNutBam() {
  // --- NÚT 3 (D8) ---
  if (digitalRead(BTN_3) == LOW && (millis() - lastPress3 > debounceTime)) {
    lastPress3 = millis();
    kichHoatBipNgan();

    if (currentMode == 0) {
      currentMode = 1;
      selectedProfileIndex = currentProfileIndex;
      lcd.clear();
    } 
    else if (currentMode == 1) {
      selectedProfileIndex = (selectedProfileIndex + 1) % 7;
      lcd.clear();
    } 
    else if (currentMode == 2) {
      if (editStep == 0) {
        editTemp -= 0.1;
        if (editTemp < 30.0) editTemp = 30.0;
      } else {
        editHumi -= 1;
        if (editHumi < 30) editHumi = 30;
      }
    }
  }

  // --- NÚT 1 (D6) ---
  if (digitalRead(BTN_1) == LOW && (millis() - lastPress1 > debounceTime)) {
    lastPress1 = millis();

    if (currentMode == 1) {
      kichHoatBipNgan();
      currentMode = 2;
      editStep = 0;
      editTemp = PROFILES_MAC_DINH[selectedProfileIndex].nhietAp;
      editHumi = PROFILES_MAC_DINH[selectedProfileIndex].doAmAp;
      lcd.clear();
    } 
    else if (currentMode == 2) {
      if (editStep == 0) {
        kichHoatBipNgan();
        editStep = 1;
        lcd.clear();
      } else {
        targetTemp = editTemp;
        targetHumi = editHumi;
        if (ngayHienTai >= currentProfile.ngayBatDauNo) {
            currentProfile.nhietNo = editTemp;
            currentProfile.doAmNo = editHumi;
        } else {
            currentProfile.nhietAp = editTemp;
            currentProfile.doAmAp = editHumi;
        }
        currentMode = 0;
        lcd.clear();
        kichHoatBipDoi(); // Lưu thành công -> 2 Bíp
      }
    }
  }

  // --- NÚT 2 (D7) ---
  if (digitalRead(BTN_2) == LOW && (millis() - lastPress2 > debounceTime)) {
    lastPress2 = millis();

    if (currentMode == 1) {
      currentProfileIndex = selectedProfileIndex;
      currentProfile = PROFILES_MAC_DINH[currentProfileIndex];
      capNhatGiaiDoanAp();
      currentMode = 0;
      lcd.clear();
      kichHoatBipDoi(); // Chọn hồ sơ thành công -> 2 Bíp
    } 
    else if (currentMode == 2) {
      kichHoatBipNgan(); 
      if (editStep == 0) {
        editTemp += 0.1;
        if (editTemp > 45.0) editTemp = 45.0;
      } else {
        editHumi += 1;
        if (editHumi > 99) editHumi = 99;
      }
    } else {
      kichHoatBipNgan();
    }
  }
}

void capNhatLCD() {
  if (currentMode == 0) {
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(nhietDo, 1);
    lcd.print(" H:");
    lcd.print((int)doAm);
    lcd.print(" ");
    lcd.print(ngayHienTai);
    lcd.print("/");
    lcd.print(tongNgayAp);
    lcd.print("   ");

    lcd.setCursor(0, 1);
    char buf2[17];
    snprintf(buf2, sizeof(buf2), "[%-4s] D:%-2s Q:%-2s",
             PROFILES_MAC_DINH[currentProfileIndex].maTen,
             trangThaiDen ? "ON" : "OF",
             trangThaiQuat ? "ON" : "OF");
    lcd.print(buf2);
  } 
  else if (currentMode == 1) {
    lcd.setCursor(0, 0);
    lcd.print("LOAI: ");
    lcd.print(PROFILES_MAC_DINH[selectedProfileIndex].ten);
    lcd.print("    ");

    lcd.setCursor(0, 1);
    lcd.print("1:Next 2:OK 3:Edit");
  } 
  else if (currentMode == 2) {
    if (editStep == 0) {
      lcd.setCursor(0, 0);
      lcd.print("CHINH NHIET DO:");
      lcd.setCursor(0, 1);
      lcd.print("Set: ");
      lcd.print(editTemp, 1);
      lcd.print(" C (OK:1)");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("CHINH DO AM:");
      lcd.setCursor(0, 1);
      lcd.print("Set: ");
      lcd.print(editHumi);
      lcd.print(" % (OK:1) ");
    }
  }
}

void guiDuLieuESP() {
  int stage = (ngayHienTai >= currentProfile.ngayBatDauNo) ? 1 : 0;
  String packet = "T:" + String(nhietDo, 1) + 
                  ",H:" + String((int)doAm) + 
                  ",Day:" + String(ngayHienTai) + 
                  ",L:" + String(trangThaiDen ? 1 : 0) + 
                  ",F:" + String(trangThaiQuat ? 1 : 0) + 
                  ",P:" + String(currentProfileIndex) + 
                  ",S:" + String(stage) + 
                  ",TT:" + String(targetTemp, 1) + 
                  ",TH:" + String(targetHumi);
  espSerial.println(packet);
}
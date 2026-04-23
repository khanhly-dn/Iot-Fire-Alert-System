#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define WIFI_SSID   "Khanh"
#define WIFI_PASS   "@133057k"

#define PIN_DHT      4
#define PIN_LED      2
#define PIN_BUZZER   5
#define DHT_TYPE    DHT11

#define OLED_W    128
#define OLED_H     64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 oled(OLED_W, OLED_H, &Wire, -1);

#define TEMP_THRESHOLD  35.0
#define HUMI_THRESHOLD  90.0

WebServer server(80);
DHT dht(PIN_DHT, DHT_TYPE);

float    temperature = 0;
float    humidity    = 0;
bool     alertActive = false;
bool     silenced    = false;
bool     buzzState   = false;
String   alertLevel  = "SAFE";
float    maxTemp     = -999;
float    minTemp     =  999;
unsigned long uptimeStart = 0;
unsigned long lastRead    = 0;
unsigned long lastBuzz    = 0;

const unsigned long READ_INTERVAL = 2000;

#define LOG_SIZE 20
struct LogEntry { unsigned long ts; float temp; float humi; String level; };
LogEntry eventLog[LOG_SIZE];
int logHead  = 0;
int logCount = 0;

void addLog(float t, float h, String lv) {
  eventLog[logHead] = { millis(), t, h, lv };
  logHead = (logHead + 1) % LOG_SIZE;
  if (logCount < LOG_SIZE) logCount++;
}

// Ham tien ich: gui CORS headers cho moi response
void sendCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
  server.sendHeader("Access-Control-Max-Age", "600");
}

// Handle OPTIONS preflight cho bat ky route nao
void handleOptions() {
  sendCORS();
  server.send(204);
}
//   OLED
void oledSafe(float t, float h) {
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);  oled.println("IoT SENSOR MONITOR");
  oled.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  oled.setCursor(0, 14); oled.print("Temp : "); oled.print(t, 1); oled.println(" C");
  oled.setCursor(0, 26); oled.print("Humi : "); oled.print(h, 1); oled.println(" %");
  oled.drawLine(0, 38, 128, 38, SSD1306_WHITE);
  oled.setCursor(0, 42); oled.println("Status : SAFE");
  oled.setCursor(0, 54); oled.println("System OK");
  oled.display();
}

void oledWarning(float t, float h) {
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);  oled.println("!! CANH BAO !!");
  oled.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  oled.setCursor(0, 14); oled.print("Temp : "); oled.print(t, 1); oled.println(" C");
  oled.setCursor(0, 26); oled.print("Humi : "); oled.print(h, 1); oled.println(" %");
  oled.setCursor(0, 42); oled.println("Kiem tra ngay!");
  oled.display();
}

void oledDanger(float t, float h) {
  static bool blink = false;
  blink = !blink;
  oled.clearDisplay();
  if (blink) {
    oled.fillRect(0, 0, 128, 14, SSD1306_WHITE);
    oled.setTextColor(SSD1306_BLACK);
  } else {
    oled.setTextColor(SSD1306_WHITE);
  }
  oled.setTextSize(1);
  oled.setCursor(2, 3);  oled.println("!! NGUY HIEM !!");
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 18); oled.print("Temp: "); oled.print(t, 1); oled.println(" C");
  oled.setCursor(0, 30); oled.print("Humi: "); oled.print(h, 1); oled.println(" %");
  oled.setCursor(0, 50); oled.println("VUOT NGUONG AN TOAN!");
  oled.display();
}
//   BUZZER
void buzzAlert(String level) {
  if (silenced) return;
  unsigned long now = millis();
  unsigned long interval = (level == "DANGER") ? 200 : 600;
  if (now - lastBuzz >= interval) {
    lastBuzz  = now;
    buzzState = !buzzState;
    digitalWrite(PIN_BUZZER, buzzState ? HIGH : LOW);
  }
}

void buzzOff() {
  buzzState = false;
  digitalWrite(PIN_BUZZER, LOW);
}
//   BUILD JSON
String buildStatus() {
  unsigned long up = millis() - uptimeStart;
  String j = "{";
  j += "\"temp\":"     + String(temperature, 2) + ",";
  j += "\"humi\":"     + String(humidity, 2)    + ",";
  j += "\"alert\":"    + String(alertActive ? "true" : "false") + ",";
  j += "\"silenced\":" + String(silenced    ? "true" : "false") + ",";
  j += "\"level\":\""  + alertLevel + "\",";
  j += "\"uptime\":"   + String(up)         + ",";
  j += "\"maxTemp\":"  + String(maxTemp, 2) + ",";
  j += "\"minTemp\":"  + String(minTemp, 2) + ",";
  j += "\"rssi\":"     + String(WiFi.RSSI()) + ",";
  j += "\"logs\":[";
  bool first = true;
  for (int i = 0; i < LOG_SIZE; i++) {
    int idx = (logHead - 1 - i + LOG_SIZE) % LOG_SIZE;
    if (eventLog[idx].ts == 0) continue;
    if (!first) j += ",";
    first = false;
    j += "{\"ts\":"      + String(eventLog[idx].ts)      +
         ",\"temp\":"    + String(eventLog[idx].temp, 2) +
         ",\"humi\":"    + String(eventLog[idx].humi, 2) +
         ",\"level\":\"" + eventLog[idx].level + "\"}";
  }
  j += "]}";
  return j;
}
//   WIFI
void connectWiFi() {
  Serial.print("Connecting WiFi: "); Serial.println(WIFI_SSID);
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE); oled.setTextSize(1);
  oled.setCursor(10, 18); oled.println("Connecting WiFi...");
  oled.setCursor(10, 32); oled.println(WIFI_SSID);
  oled.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
    attempts++;
    if (attempts > 60) { Serial.println("\nTimeout! Restarting..."); ESP.restart(); }
  }
  Serial.print("\nConnected! IP: "); Serial.println(WiFi.localIP());
  oled.clearDisplay();
  oled.setCursor(0, 10); oled.println("WiFi Connected!");
  oled.setCursor(0, 24); oled.println(WiFi.localIP().toString());
  oled.setCursor(0, 40); oled.println("Mo index.html");
  oled.setCursor(0, 52); oled.println("tren trinh duyet");
  oled.display();
  delay(1500);
}
//   SETUP
void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED,    OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_LED,    LOW);
  digitalWrite(PIN_BUZZER, LOW);

  dht.begin();

  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found!");
  }
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE); oled.setTextSize(1);
  oled.setCursor(10, 20); oled.println("IoT Sensor Monitor");
  oled.setCursor(30, 36); oled.println("Starting...");
  oled.display();
  delay(1000);

  connectWiFi();
  uptimeStart = millis();

  // CORS: KHONG dung enableCORS(true) vi no xung dot voi sendCORS() thu cong
  // Thay vao do dung handleOptions() cho tung route

  // ---- /status ----
  server.on("/status", HTTP_OPTIONS, handleOptions);
  server.on("/status", HTTP_GET, []() {
    sendCORS();
    server.send(200, "application/json", buildStatus());
  });

  // ---- /info ----
  server.on("/info", HTTP_OPTIONS, handleOptions);
  server.on("/info", HTTP_GET, []() {
    sendCORS();
    String j = "{\"ip\":\"" + WiFi.localIP().toString() + "\",";
    j += "\"ssid\":\"" + String(WIFI_SSID) + "\",";
    j += "\"rssi\":"   + String(WiFi.RSSI()) + "}";
    server.send(200, "application/json", j);
  });

  // ---- /test ----
  server.on("/test", HTTP_OPTIONS, handleOptions);
  server.on("/test", HTTP_GET, []() {
    sendCORS();
    alertActive = true; alertLevel = "DANGER"; silenced = false;
    digitalWrite(PIN_LED, HIGH);
    addLog(temperature, humidity, "DANGER");
    server.send(200, "text/plain", "Test triggered");
  });

  // ---- /silence ----
  server.on("/silence", HTTP_OPTIONS, handleOptions);
  server.on("/silence", HTTP_GET, []() {
    sendCORS();
    silenced = true; buzzOff();
    server.send(200, "text/plain", "Silenced");
  });

  // ---- /reset ----
  server.on("/reset", HTTP_OPTIONS, handleOptions);
  server.on("/reset", HTTP_GET, []() {
    sendCORS();
    alertActive = false; silenced = false; alertLevel = "SAFE";
    maxTemp = -999; minTemp = 999;
    logHead = 0; logCount = 0;
    memset(eventLog, 0, sizeof(eventLog));
    digitalWrite(PIN_LED, LOW); buzzOff();
    server.send(200, "text/plain", "Reset OK");
  });

  server.begin();
  Serial.print("API ready at: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/status");
}
//   LOOP
void loop() {
  server.handleClient();

  unsigned long now = millis();
  if (now - lastRead >= READ_INTERVAL) {
    lastRead = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      temperature = t;
      humidity    = h;
      if (t > maxTemp) maxTemp = t;
      if (t < minTemp) minTemp = t;

      String prevLevel = alertLevel;
      if (t >= TEMP_THRESHOLD + 5 || h >= HUMI_THRESHOLD + 10) {
        alertLevel = "DANGER";  alertActive = true;
      } else if (t >= TEMP_THRESHOLD || h >= HUMI_THRESHOLD) {
        alertLevel = "WARNING"; alertActive = true;
      } else {
        alertLevel = "SAFE"; alertActive = false; silenced = false;
      }

      if (alertLevel != prevLevel) addLog(t, h, alertLevel);

      digitalWrite(PIN_LED, alertActive ? HIGH : LOW);
      if (alertActive) buzzAlert(alertLevel);
      else             buzzOff();

      if      (alertLevel == "DANGER")  oledDanger(t, h);
      else if (alertLevel == "WARNING") oledWarning(t, h);
      else                              oledSafe(t, h);

      Serial.print("Temp: "); Serial.print(t);
      Serial.print(" C | Humi: "); Serial.print(h);
      Serial.print(" % | "); Serial.println(alertLevel);
    } else {
      Serial.println("DHT11 read failed!");
    }
  }
}

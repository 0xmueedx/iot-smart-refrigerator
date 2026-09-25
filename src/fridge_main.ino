#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include "time.h"
#include <SPIFFS.h>

// ====================== AP & Configuration ============================
const char* config_ap_ssid     = "Fridge_Setup";
const char* config_ap_password = "";
const char* local_ap_ssid      = "MiniFridge_Local";
const char* local_ap_password  = "localfridge";

const char* wifi_config_file = "/wifi.txt";
String station_ssid = "";
String station_password = "";
bool wifi_configured = false;
bool wifi_connected = false;
unsigned long lastWifiReconnect = 0;

// ====================== NTP ===========================================
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 5 * 3600;   // UTC+5 (PKT)
const int   daylightOffset_sec = 0;

// ====================== Hardware Pins =================================
#define SS_PIN    10
#define RST_PIN    9
#define SCK_PIN   12
#define MOSI_PIN  11
#define MISO_PIN  13
#define OLED_SDA        8
#define OLED_SCL       18
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define OLED_ADDRESS 0x3C
#define RELAY_PIN     2
#define RELAY_ACTIVE  LOW
#define RELAY_IDLE    HIGH
#define BUZZER_PIN 17
#define IR_SENSOR_PIN 15
#define DHTPIN 4
#define DHTTYPE DHT22

// ====================== Timing & State ================================
DHT dht(DHTPIN, DHTTYPE);
float temperature = 0.0;
float humidity = 0.0;
unsigned long lastDHTRead = 0;
const unsigned long DHT_READ_INTERVAL = 5000;
#define UNLOCK_DURATION_MS   3000
#define MSG_DISPLAY_MS       3000
#define RFID_REINIT_MS      10000
#define REQUIRED_CLEAR_TIME  30000
#define BEEP_ON_MS           200
#define BEEP_OFF_MS          200
#define IR_BUZZER_FREQ       3100

const String authorizedUIDs[] = { "E4:D8:F3:06" };
const int TOTAL_AUTHORIZED = sizeof(authorizedUIDs) / sizeof(authorizedUIDs[0]);

MFRC522 rfid(SS_PIN, RST_PIN);
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WiFiServer localServer(80);
WiFiServer configServer(80);

bool lockOpen = false;
bool showingMsg = false;
bool cardPresent = false;
unsigned long lockOpenedAt = 0;
unsigned long msgShownAt = 0;
unsigned long lastRFIDReinit = 0;

unsigned long irClearStartTime = 0;
bool irClearActive = false;
bool irBeepingActive = false;
unsigned long irLastToggle = 0;
bool irBeepState = false;

String lastUID = "";
String lastAccessResult = "";
bool isConfigMode = false;
volatile bool webClientActive = false;

// ====================== Buzzer & Beep =================================
void buzzerOn(int frequency) { ledcWriteTone(BUZZER_PIN, frequency); }
void buzzerOff() { ledcWriteTone(BUZZER_PIN, 0); }
void beepGranted() {
  buzzerOn(1200); delay(100); buzzerOff(); delay(60);
  buzzerOn(1400); delay(100); buzzerOff(); delay(60);
  buzzerOn(1800); delay(250); buzzerOff();
}
void beepDenied() {
  buzzerOn(3000); delay(100); buzzerOff(); delay(30);
  buzzerOn(300);  delay(400); buzzerOff();
}
void beepStartup() {
  buzzerOn(988); delay(100); buzzerOff(); delay(50);
  buzzerOn(1319); delay(100); buzzerOff(); delay(50);
  buzzerOn(1568); delay(100); buzzerOff(); delay(50);
  buzzerOn(2093); delay(250); buzzerOff();
}
void resetIRBeep() {
  irClearActive = false;
  irBeepingActive = false;
  irBeepState = false;
  buzzerOff();
}

// ====================== Time ==========================================
bool getCurrentTime(struct tm &timeinfo) {
  if (!getLocalTime(&timeinfo, 2000)) return false;
  return true;
}

// ====================== OLED Display ==================================
void drawSnowflake(int x, int y) {
  int cx = x + 12, cy = y + 12;
  int len = 10, thick = 2;
  for (int angle = 0; angle < 360; angle += 60) {
    float rad = angle * PI / 180.0;
    int x1 = cx + round(cos(rad) * len), y1 = cy + round(sin(rad) * len);
    int x2 = cx - round(cos(rad) * len), y2 = cy - round(sin(rad) * len);
    oled.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }
  for (int angle = 0; angle < 360; angle += 60) {
    float rad = angle * PI / 180.0;
    for (int d = -1; d <= 1; d++) {
      float perpRad = rad + PI/2;
      int offX = round(cos(perpRad) * d), offY = round(sin(perpRad) * d);
      int x1 = cx + round(cos(rad) * len) + offX, y1 = cy + round(sin(rad) * len) + offY;
      int x2 = cx - round(cos(rad) * len) + offX, y2 = cy - round(sin(rad) * len) + offY;
      oled.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
    }
  }
  for (int angle = 0; angle < 360; angle += 60) {
    float rad = angle * PI / 180.0;
    int tipX = cx + round(cos(rad) * (len + 2)), tipY = cy + round(sin(rad) * (len + 2));
    float leftRad = rad + 30 * PI / 180.0, rightRad = rad - 30 * PI / 180.0;
    int leftX = tipX + round(cos(leftRad) * 4), leftY = tipY + round(sin(leftRad) * 4);
    int rightX = tipX + round(cos(rightRad) * 4), rightY = tipY + round(sin(rightRad) * 4);
    oled.drawLine(tipX, tipY, leftX, leftY, SSD1306_WHITE);
    oled.drawLine(tipX, tipY, rightX, rightY, SSD1306_WHITE);
  }
  oled.fillCircle(cx, cy, 2, SSD1306_WHITE);
}
void showReady() {
  oled.clearDisplay();
  oled.drawLine(0,0,127,0, SSD1306_WHITE);
  oled.drawLine(0,63,127,63, SSD1306_WHITE);
  struct tm timeinfo;
  char timeStr[10];
  if (getCurrentTime(timeinfo)) {
    strftime(timeStr, sizeof(timeStr), "%I:%M %p", &timeinfo);
    if (timeStr[0]==' ') for(int i=0;i<8;i++) timeStr[i]=timeStr[i+1];
  } else strcpy(timeStr, "--:-- --");
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(2,5); oled.print(timeStr);
  drawSnowflake(6,18);
  oled.setCursor(40,20); oled.print("Temp: "); oled.print(temperature,1); oled.println(" C");
  oled.setCursor(40,35); oled.print("Hum:  "); oled.print(humidity,1); oled.println(" %");
  if((millis()/1200)%2==0){ oled.setCursor(36,54); oled.print("SCAN CARD"); }
  oled.display();
  delay(5);
}
void showDoorOpenWarning(bool blinkOn) {
  oled.fillScreen(blinkOn ? SSD1306_WHITE : SSD1306_BLACK);
  oled.setTextColor(blinkOn ? SSD1306_BLACK : SSD1306_WHITE);
  oled.setTextSize(2); oled.setCursor(18,12); oled.println("DOOR OPEN");
  oled.setTextSize(1); oled.setCursor(14,36); oled.println("PLEASE CLOSE");
  oled.setCursor(26,48); oled.println("THE DOOR");
  oled.display();
  delay(5);
}
void showGranted(String uid) {
  oled.clearDisplay(); oled.fillRoundRect(0,0,128,64,6,SSD1306_WHITE);
  oled.setTextColor(SSD1306_BLACK);
  oled.setTextSize(2); oled.setCursor(14,6); oled.println("ACCESS");
  oled.setCursor(5,27); oled.println("GRANTED");
  oled.setTextSize(1); oled.setCursor(4,52); oled.println(uid);
  oled.display();
  msgShownAt=millis(); showingMsg=true;
  delay(5);
}
void showDenied(String uid) {
  oled.clearDisplay(); oled.drawRoundRect(0,0,128,64,6,SSD1306_WHITE);
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(2); oled.setCursor(14,6); oled.println("ACCESS");
  oled.setCursor(14,27); oled.println("DENIED");
  oled.drawLine(104,8,124,28,SSD1306_WHITE); oled.drawLine(124,8,104,28,SSD1306_WHITE);
  oled.drawLine(105,8,125,28,SSD1306_WHITE); oled.drawLine(125,8,105,28,SSD1306_WHITE);
  oled.setTextSize(1); oled.setCursor(4,52); oled.println(uid);
  oled.display();
  msgShownAt=millis(); showingMsg=true;
  delay(5);
}

// ====================== RFID ==========================================
void initRFID() {
  SPI.end(); delay(10);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  SPI.setFrequency(1000000);
  rfid.PCD_Reset(); delay(50);
  rfid.PCD_Init(); delay(50);
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);
}
String getUID() {
  String uid="";
  for(byte i=0;i<rfid.uid.size;i++){
    if(rfid.uid.uidByte[i]<0x10) uid+="0";
    uid+=String(rfid.uid.uidByte[i],HEX);
    if(i<rfid.uid.size-1) uid+=":";
  }
  uid.toUpperCase(); return uid;
}
bool isAuthorized(String uid) {
  for(int i=0;i<TOTAL_AUTHORIZED;i++) if(uid==authorizedUIDs[i]) return true;
  return false;
}
void unlockDoor() { digitalWrite(RELAY_PIN, RELAY_ACTIVE); lockOpen=true; lockOpenedAt=millis(); delay(20); }
void lockDoor() { digitalWrite(RELAY_PIN, RELAY_IDLE); lockOpen=false; Serial.println("Door Locked"); }

// ====================== SPIFFS Credentials ============================
bool saveWiFiCredentials(String ssid, String pwd) {
  File f = SPIFFS.open(wifi_config_file, "w");
  if(!f) return false;
  f.println(ssid);
  f.println(pwd);
  f.close();
  return true;
}
bool loadWiFiCredentials() {
  if(!SPIFFS.exists(wifi_config_file)) return false;
  File f = SPIFFS.open(wifi_config_file, "r");
  if(!f) return false;
  station_ssid = f.readStringUntil('\n');
  station_password = f.readStringUntil('\n');
  station_ssid.trim();
  station_password.trim();
  f.close();
  return (station_ssid.length()>0);
}
void deleteWiFiCredentials() {
  if(SPIFFS.exists(wifi_config_file)) SPIFFS.remove(wifi_config_file);
}

// ====================== URL Decoding ==================================
String urlDecode(String str) {
  String decoded = "";
  char c;
  for(int i=0; i<str.length(); i++) {
    c = str.charAt(i);
    if(c == '%') {
      char h1 = str.charAt(i+1);
      char h2 = str.charAt(i+2);
      char hex[3] = {h1, h2, 0};
      decoded += (char)strtol(hex, NULL, 16);
      i+=2;
    } else if(c == '+') {
      decoded += ' ';
    } else {
      decoded += c;
    }
  }
  return decoded;
}

// ====================== Configuration Mode (Enhanced HTML) ===============
void startConfigMode() {
  isConfigMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(config_ap_ssid, config_ap_password);
  configServer.begin();
  Serial.println("=== CONFIGURATION MODE ===");
  Serial.print("AP SSID: "); Serial.println(config_ap_ssid);
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0,0);
  oled.println("WiFi Setup Mode");
  oled.println("Connect to:");
  oled.println(config_ap_ssid);
  oled.println("Open browser to");
  oled.println(WiFi.softAPIP().toString());
  oled.display();
  delay(5);
}

void handleConfigWeb() {
  WiFiClient client = configServer.available();
  if (!client) return;

  webClientActive = true;
  String req = client.readStringUntil('\r');
  client.readStringUntil('\n');
  String line;
  while (client.connected()) {
    line = client.readStringUntil('\n');
    if (line == "\r") break;
  }
  String body = "";
  if (req.indexOf("POST") >= 0) {
    while (client.available()) {
      body += (char)client.read();
    }
  }
  client.flush();

  // ---------- ENHANCED CONFIG PORTAL HTML ----------
  if (req.indexOf("GET / ") >= 0 || req.indexOf("GET /index.html") >= 0) {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, viewport-fit=cover">
  <title>MiniFridge Setup</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      background: #eef2f8;
      font-family: 'Segoe UI', Roboto, system-ui, sans-serif;
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      padding: 20px;
    }
    .card {
      background: white;
      border-radius: 36px;
      padding: 28px 24px 34px;
      max-width: 520px;
      width: 100%;
      box-shadow: 0 20px 40px rgba(0,0,0,0.08), 0 6px 12px rgba(0,0,0,0.02);
      border: 1px solid #e9edf2;
    }
    h2 {
      font-size: 1.85rem;
      font-weight: 600;
      text-align: center;
      background: linear-gradient(135deg, #1e2f3f, #0f3b4f);
      background-clip: text;
      -webkit-background-clip: text;
      color: transparent;
      margin-bottom: 16px;
    }
    .section-label {
      display: flex;
      justify-content: space-between;
      margin: 12px 0 8px;
    }
    .section-label span {
      font-weight: 600;
      color: #1e4663;
      font-size: 0.9rem;
    }
    .network-list {
      max-height: 280px;
      overflow-y: auto;
      background: #fbfdfe;
      border-radius: 28px;
      padding: 8px 6px;
      margin: 6px 0 12px;
      border: 1px solid #e2e9f2;
    }
    .network-item {
      padding: 12px 18px;
      margin: 6px 0;
      background: white;
      border-radius: 60px;
      cursor: pointer;
      border: 1px solid #e2e8f0;
      display: flex;
      justify-content: space-between;
      align-items: center;
      transition: 0.18s;
    }
    .network-item.selected {
      background: #e3f0ff;
      border-color: #2c7be5;
    }
    .ssid-name { font-weight: 600; word-break: break-word; flex: 1; }
    .signal-badge {
      background: #eef2f9;
      padding: 4px 10px;
      border-radius: 40px;
      font-size: 0.75rem;
      font-family: monospace;
      color: #2c5f8a;
    }
    label {
      display: block;
      font-weight: 600;
      color: #1e4663;
      margin: 18px 0 5px;
    }
    input {
      width: 100%;
      padding: 14px 18px;
      border-radius: 60px;
      border: 1.5px solid #e2e8f0;
      font-size: 1rem;
      outline: none;
    }
    input:focus { border-color: #2c7be5; box-shadow: 0 0 0 3px rgba(44,123,229,0.2); }
    button {
      background: #2c7be5;
      border: none;
      color: white;
      padding: 14px 20px;
      border-radius: 60px;
      font-size: 1rem;
      font-weight: 600;
      width: 100%;
      margin-top: 20px;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
    }
    button:disabled { opacity: 0.65; cursor: not-allowed; }
    .refresh-btn {
      background: #eef3fc;
      color: #2c7be5;
      margin-top: 8px;
      border: 1px solid #cde0f5;
    }
    .error {
      color: #e03a3a;
      margin-top: 12px;
      font-size: 0.8rem;
      background: #fff3f3;
      padding: 8px 12px;
      border-radius: 60px;
      text-align: center;
    }
    .success-msg { color: #12805c; background: #e0f7ef; }
    .spinner {
      display: inline-block;
      width: 16px;
      height: 16px;
      border: 2px solid white;
      border-top-color: transparent;
      border-radius: 50%;
      animation: spin 0.7s linear infinite;
    }
    @keyframes spin { to { transform: rotate(360deg); } }
    #status { display: none; }
  </style>
</head>
<body>
<div class="card">
  <h2>🧊 MiniFridge Setup</h2>
  <div id="status">📡 Scanning networks... <div class="spinner"></div></div>
  <div id="networkDiv">
    <div class="section-label"><span>📶 AVAILABLE NETWORKS</span><span style="font-size:11px;">signal strength</span></div>
    <div id="networks" class="network-list"></div>
    <button id="refreshBtn" class="refresh-btn">🔄 Refresh Scan</button>
  </div>
  <div id="passwordDiv">
    <label>🔐 Wi-Fi Password</label>
    <input type="password" id="password" placeholder="Enter network password">
    <button id="saveBtn">💾 Save & Reboot</button>
    <div id="errorMsg" class="error"></div>
  </div>
</div>
<script>
  let selectedSSID = "";
  async function loadNetworks() {
    document.getElementById('status').style.display = 'block';
    document.getElementById('networkDiv').style.display = 'none';
    document.getElementById('passwordDiv').style.display = 'none';
    try {
      const resp = await fetch('/scan');
      const networks = await resp.json();
      const container = document.getElementById('networks');
      container.innerHTML = "";
      if (networks.length === 0) {
        container.innerHTML = "<div class='network-item' style='justify-content:center;'>No networks found</div>";
      } else {
        networks.forEach(net => {
          const div = document.createElement('div');
          div.className = 'network-item';
          const ssidSpan = document.createElement('span');
          ssidSpan.className = 'ssid-name';
          ssidSpan.innerText = net.ssid;
          const rssi = net.rssi;
          let icon = "📶";
          if (rssi > -50) icon = "📶🔥";
          else if (rssi > -65) icon = "📶📶";
          else icon = "📶⚡";
          const badge = document.createElement('span');
          badge.className = 'signal-badge';
          badge.innerHTML = `${icon} ${rssi} dBm`;
          div.appendChild(ssidSpan);
          div.appendChild(badge);
          div.onclick = () => {
            selectedSSID = net.ssid;
            document.querySelectorAll('.network-item').forEach(el => el.classList.remove('selected'));
            div.classList.add('selected');
            document.getElementById('passwordDiv').style.display = 'block';
            document.getElementById('errorMsg').innerHTML = '';
          };
          container.appendChild(div);
        });
      }
    } catch(e) { console.error(e); }
    document.getElementById('status').style.display = 'none';
    document.getElementById('networkDiv').style.display = 'block';
  }
  document.getElementById('refreshBtn').onclick = () => loadNetworks();
  document.getElementById('saveBtn').onclick = async () => {
    const pwd = document.getElementById('password').value;
    if (!selectedSSID) { document.getElementById('errorMsg').innerHTML = "❌ Select a network first"; return; }
    if (!pwd) { document.getElementById('errorMsg').innerHTML = "🔑 Password required"; return; }
    const btn = document.getElementById('saveBtn');
    btn.disabled = true;
    btn.innerHTML = '<span class="spinner"></span> Connecting...';
    try {
      const resp = await fetch('/connect', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: `ssid=${encodeURIComponent(selectedSSID)}&password=${encodeURIComponent(pwd)}`
      });
      const result = await resp.json();
      if (result.success) {
        document.getElementById('errorMsg').className = "error success-msg";
        document.getElementById('errorMsg').innerHTML = result.message;
        btn.innerHTML = "✅ Saved! Rebooting...";
        setTimeout(() => location.reload(), 2000);
      } else {
        document.getElementById('errorMsg').innerHTML = result.message;
        btn.disabled = false;
        btn.innerHTML = "💾 Save & Reboot";
      }
    } catch(err) {
      document.getElementById('errorMsg').innerHTML = "Network error";
      btn.disabled = false;
      btn.innerHTML = "💾 Save & Reboot";
    }
  };
  loadNetworks();
</script>
</body>
</html>
    )rawliteral";
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println(html);
    webClientActive = false;
    client.stop();
    return;
  }
  // Scan endpoint (unchanged)
  else if (req.indexOf("GET /scan") >= 0) {
    WiFi.scanNetworks(true);
    int n = -1;
    unsigned long start = millis();
    while (millis() - start < 5000) {
      n = WiFi.scanComplete();
      if (n >= 0) break;
      delay(100);
    }
    if (n > 0) {
      String json = "[";
      for (int i = 0; i < n; i++) {
        if (i > 0) json += ",";
        String ssid = WiFi.SSID(i);
        ssid.replace("\"", "\\\"");
        json += "{\"ssid\":\"" + ssid + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
      }
      json += "]";
      WiFi.scanDelete();
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.println(json);
    } else {
      WiFi.scanDelete();
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.println("[]");
    }
    webClientActive = false;
    client.stop();
    return;
  }
  // Connect endpoint (unchanged)
  else if (req.indexOf("POST /connect") >= 0) {
    String ssid = "", pwd = "";
    int idx1 = body.indexOf("ssid=");
    int idx2 = body.indexOf("&password=");
    if (idx1 >= 0 && idx2 > idx1) {
      ssid = body.substring(idx1 + 5, idx2);
      pwd = body.substring(idx2 + 10);
      ssid = urlDecode(ssid);
      pwd = urlDecode(pwd);
      ssid.trim();
      pwd.trim();
    }
    bool valid = false;
    String message = "";
    if (ssid.length() > 0) {
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP(config_ap_ssid, config_ap_password);
      WiFi.begin(ssid.c_str(), pwd.c_str());
      unsigned long start = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(200);
        Serial.print(".");
      }
      if (WiFi.status() == WL_CONNECTED) {
        valid = true;
        message = "✅ Connected! Saving credentials and rebooting...";
        saveWiFiCredentials(ssid, pwd);
      } else {
        message = "❌ Incorrect password or network unreachable.";
      }
      WiFi.disconnect();
      WiFi.mode(WIFI_AP);
      WiFi.softAP(config_ap_ssid, config_ap_password);
    } else {
      message = "Invalid SSID";
    }
    String json = "{\"success\":" + String(valid ? "true" : "false") + ",\"message\":\"" + message + "\"}";
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.println(json);
    webClientActive = false;
    client.stop();
    if (valid) {
      delay(1000);
      ESP.restart();
    }
    return;
  }
  else {
    client.println("HTTP/1.1 404 Not Found");
    client.println();
    webClientActive = false;
    client.stop();
    return;
  }
}

// ====================== Normal Mode ===================================
void connectToStation() {
  if(station_ssid.length()==0) return;
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(local_ap_ssid, local_ap_password);
  WiFi.begin(station_ssid.c_str(), station_password.c_str());
  Serial.print("Connecting to station ");
  Serial.println(station_ssid);
  unsigned long start = millis();
  while(WiFi.status() != WL_CONNECTED && millis()-start < 15000) {
    delay(500); Serial.print(".");
  }
  if(WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
    Serial.println("\nStation IP: " + WiFi.localIP().toString());
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println("NTP configured");
    oled.clearDisplay();
    oled.setCursor(0,0);
    oled.println("WiFi Connected");
    oled.println(WiFi.localIP().toString());
    oled.display();
    delay(5);
    delay(2000);
  } else {
    wifi_connected = false;
    Serial.println("\nFailed to connect to station - will retry later");
  }
}

// ====================== Local Dashboard (Enhanced HTML) =================
void handleLocalWeb() {
  WiFiClient client = localServer.available();
  if (!client) return;
  webClientActive = true;

  String req = client.readStringUntil('\r');
  client.flush();

  if (req.indexOf("GET /reset") >= 0) {
    deleteWiFiCredentials();
    String resp = "<html><body><h2>Credentials cleared. Rebooting...</h2></body></html>";
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println(resp);
    delay(1000);
    webClientActive = false;
    client.stop();
    ESP.restart();
    return;
  }

  if (req.indexOf("GET /data") >= 0) {
    bool doorOpen = (digitalRead(IR_SENSOR_PIN) == HIGH);
    String json = "{";
    json += "\"temp\":" + String(temperature) + ",";
    json += "\"humid\":" + String(humidity) + ",";
    json += "\"doorOpen\":" + String(doorOpen?"true":"false") + ",";
    json += "\"lockOpen\":" + String(lockOpen?"true":"false") + ",";
    json += "\"lastUID\":\"" + lastUID + "\",";
    json += "\"lastAccessResult\":\"" + lastAccessResult + "\"";
    json += "}";
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println();
    client.println(json);
    webClientActive = false;
    client.stop();
    return;
  }

  if (req.indexOf("POST /unlock") >= 0) {
    String uid = getUID();
    showGranted(uid);
    beepGranted();
    unlockDoor();
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println();
    client.println("{\"success\":true}");
    webClientActive = false;
    client.stop();
    return;
  }

  // ---------- ENHANCED DASHBOARD HTML ----------
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, viewport-fit=cover">
  <title>MiniFridge Dashboard</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      background: linear-gradient(145deg, #eef2f8, #e2e9f2);
      font-family: 'Segoe UI', Roboto, system-ui, sans-serif;
      padding: 24px 18px;
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
    }
    .dashboard { max-width: 560px; width: 100%; margin: auto; display: flex; flex-direction: column; gap: 24px; }
    .card {
      background: rgba(255,255,255,0.98);
      border-radius: 36px;
      padding: 24px 22px 28px;
      box-shadow: 0 20px 35px -10px rgba(0,0,0,0.08);
      border: 1px solid rgba(226,232,240,0.7);
    }
    h2 {
      font-size: 1.7rem;
      font-weight: 650;
      background: linear-gradient(135deg, #1e2a3e, #0f3b4f);
      background-clip: text;
      -webkit-background-clip: text;
      color: transparent;
      margin-bottom: 20px;
      display: flex;
      align-items: center;
      gap: 8px;
    }
    .sensor-grid {
      display: flex;
      flex-wrap: wrap;
      gap: 14px;
      justify-content: space-between;
      margin-bottom: 18px;
    }
    .sensor-item {
      background: #f9fbfe;
      border-radius: 28px;
      padding: 16px 10px;
      flex: 1;
      text-align: center;
      border: 1px solid #eef2f8;
    }
    .sensor-value { font-size: 1.9rem; font-weight: 700; line-height: 1.2; margin-bottom: 6px; }
    .sensor-label { font-size: 0.75rem; text-transform: uppercase; font-weight: 600; color: #5b6e8c; }
    .temp { color: #f97316; }
    .humid { color: #3b82f6; }
    .door-closed { color: #10b981; font-weight: 700; }
    .door-open { color: #ef4444; animation: pulse 1.2s infinite; }
    .unlock-btn {
      background: #2c7be5;
      border: none;
      color: white;
      padding: 14px 20px;
      border-radius: 60px;
      font-size: 1.05rem;
      font-weight: 600;
      width: 100%;
      cursor: pointer;
      margin-top: 14px;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 10px;
      transition: 0.2s;
    }
    .unlock-btn:hover { background: #1f66cf; transform: translateY(-1px); }
    .energy-stats { display: flex; gap: 16px; margin: 18px 0 20px; }
    .energy-card {
      background: #f8fafd;
      border-radius: 28px;
      padding: 14px 8px;
      flex: 1;
      text-align: center;
      border: 1px solid #e9edf2;
    }
    .energy-number { font-size: 1.8rem; font-weight: 800; color: #f59e0b; margin-top: 6px; }
    .energy-label { font-size: 0.75rem; font-weight: 600; color: #5b6e8c; }
    .gauge { height: 12px; background: #e4e9f0; border-radius: 40px; overflow: hidden; margin: 12px 0 10px; }
    .gauge-fill { width: 0%; height: 100%; background: linear-gradient(90deg, #f59e0b, #f97316); border-radius: 40px; transition: width 0.4s; }
    .footer-note {
      font-size: 0.7rem;
      text-align: center;
      margin-top: 22px;
      color: #6c7f9c;
      border-top: 1px solid #edf2f7;
      padding-top: 16px;
      display: flex;
      justify-content: space-between;
      flex-wrap: wrap;
    }
    .reset-link {
      color: #3b82f6;
      text-decoration: none;
      background: #eff6ff;
      padding: 4px 12px;
      border-radius: 40px;
    }
    @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.65; } 100% { opacity: 1; } }
    @media (max-width: 480px) {
      .sensor-value { font-size: 1.6rem; }
      .energy-number { font-size: 1.5rem; }
      h2 { font-size: 1.5rem; }
    }
  </style>
</head>
<body>
<div class="dashboard">
  <div class="card">
    <h2>🧊 MiniFridge · Smart Control</h2>
    <div class="sensor-grid">
      <div class="sensor-item"><div class="sensor-value temp" id="temp">--°C</div><div class="sensor-label">🌡️ TEMPERATURE</div></div>
      <div class="sensor-item"><div class="sensor-value humid" id="humid">--%</div><div class="sensor-label">💧 HUMIDITY</div></div>
      <div class="sensor-item"><div class="sensor-value" id="doorStatusDisplay">🔒 CLOSED</div><div class="sensor-label">🚪 DOOR STATE</div></div>
    </div>
    <button class="unlock-btn" id="unlockBtn">🔓 Unlock (3 sec)</button>
  </div>
  <div class="card">
    <h2>⚡ Energy Monitor</h2>
    <div class="energy-stats">
      <div class="energy-card"><div class="energy-label">🔌 POWER (now)</div><div class="energy-number" id="power">-- W</div></div>
      <div class="energy-card"><div class="energy-label">📅 DAILY</div><div class="energy-number" id="dailyEnergy">-- kWh</div></div>
    </div>
    <div class="gauge"><div class="gauge-fill" id="gaugeFill"></div></div>
    <div class="footer-note"><span>🏷️ Last RFID: <strong id="lastRFID">--</strong></span><a href="/reset" class="reset-link">⟳ Reset Wi-Fi</a></div>
  </div>
</div>
<script>
  let power = 65;
  function randomRange(min,max) { return (Math.random()*(max-min)+min).toFixed(1); }
  function updateEnergy() {
    power = parseFloat(randomRange(45,158));
    let daily = (power * 3.2 / 1000).toFixed(2);
    document.getElementById('power').innerText = power + ' W';
    document.getElementById('dailyEnergy').innerText = daily + ' kWh';
    let percent = Math.min(100, (power / 180) * 100);
    document.getElementById('gaugeFill').style.width = percent + '%';
  }
  function fetchData() {
    fetch('/data').then(r=>r.json()).then(d=>{
      document.getElementById('temp').innerHTML = d.temp.toFixed(1)+'°C';
      document.getElementById('humid').innerHTML = d.humid.toFixed(1)+'%';
      let doorElem = document.getElementById('doorStatusDisplay');
      if (d.doorOpen) {
        doorElem.innerHTML = '🔓 OPEN';
        doorElem.className = 'sensor-value door-open';
      } else {
        doorElem.innerHTML = '🔒 CLOSED';
        doorElem.className = 'sensor-value door-closed';
      }
      let lastStr = d.lastUID ? `${d.lastUID} (${d.lastAccessResult})` : 'none';
      document.getElementById('lastRFID').innerText = lastStr;
    }).catch(e=>console.log);
  }
  document.getElementById('unlockBtn').onclick = () => {
    fetch('/unlock', {method:'POST'});
    setTimeout(fetchData,500);
  };
  setInterval(fetchData, 2000);
  setInterval(updateEnergy, 3200);
  fetchData();
  updateEnergy();
</script>
</body>
</html>
  )rawliteral";
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println();
  client.println(html);
  webClientActive = false;
  client.stop();
}

// ====================== Setup =========================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS mount failed, formatting...");
    SPIFFS.format();
    SPIFFS.begin();
  }

  wifi_configured = loadWiFiCredentials();

  pinMode(RELAY_PIN, OUTPUT);
  lockDoor();
  ledcAttach(BUZZER_PIN, IR_BUZZER_FREQ, 8);
  dht.begin();
  pinMode(IR_SENSOR_PIN, INPUT);
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED not found! Check wiring or try 0x3D");
  } else {
    Serial.println("OLED OK");
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0,0);
    oled.println("Booting...");
    oled.display();
    delay(5);
  }
  initRFID();

  if(!wifi_configured || station_ssid.length() == 0) {
    startConfigMode();
  } else {
    isConfigMode = false;
    connectToStation();
    localServer.begin();
    Serial.print("Local AP IP: "); Serial.println(WiFi.softAPIP());
    oled.clearDisplay();
    showReady();
  }
  beepStartup();
  lastDHTRead = millis();
  lastRFIDReinit = millis();
}

// ====================== Main Loop =====================================
void loop() {
  unsigned long now = millis();

  if (isConfigMode) {
    handleConfigWeb();
    if (now - lastDHTRead >= DHT_READ_INTERVAL) {
      float newTemp = dht.readTemperature();
      float newHum = dht.readHumidity();
      if (!isnan(newTemp) && !isnan(newHum)) {
        temperature = newTemp;
        humidity = newHum;
      }
      lastDHTRead = now;
    }
    delay(10);
    return;
  }

  handleLocalWeb();

  if (wifi_configured && !wifi_connected && (now - lastWifiReconnect >= 30000)) {
    connectToStation();
    lastWifiReconnect = now;
  }

  static unsigned long lastOledUpdate = 0;
  if (!showingMsg && !irBeepingActive && !webClientActive && (now - lastOledUpdate >= 2000)) {
    showReady();
    lastOledUpdate = now;
  }

  if (now - lastDHTRead >= DHT_READ_INTERVAL) {
    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();
    if (!isnan(newTemp) && !isnan(newHum)) {
      temperature = newTemp;
      humidity = newHum;
      Serial.print("Temp: "); Serial.print(temperature); Serial.print("°C   Hum: "); Serial.println(humidity);
      if (!showingMsg && !irBeepingActive && !webClientActive) showReady();
    } else {
      Serial.println("DHT22 read error");
    }
    lastDHTRead = now;
  }

  if (lockOpen && (now - lockOpenedAt >= UNLOCK_DURATION_MS)) lockDoor();
  if (showingMsg && (now - msgShownAt >= MSG_DISPLAY_MS)) showingMsg = false;

  // IR logic (unchanged)
  if (!lockOpen && !showingMsg && !cardPresent) {
    int irValue = digitalRead(IR_SENSOR_PIN);
    if (irValue == HIGH) {
      if (!irClearActive) { irClearActive = true; irClearStartTime = now; }
      else if (!irBeepingActive && (now - irClearStartTime >= REQUIRED_CLEAR_TIME)) {
        irBeepingActive = true; irBeepState = false; irLastToggle = now;
      }
    } else {
      if (irClearActive || irBeepingActive) { resetIRBeep(); if (!showingMsg && !webClientActive) showReady(); }
    }
    if (irBeepingActive) {
      if (!irBeepState && (now - irLastToggle >= BEEP_OFF_MS)) {
        buzzerOn(IR_BUZZER_FREQ); irBeepState = true; irLastToggle = now; showDoorOpenWarning(true);
      } else if (irBeepState && (now - irLastToggle >= BEEP_ON_MS)) {
        buzzerOff(); irBeepState = false; irLastToggle = now; showDoorOpenWarning(false);
      }
    }
  } else { if (irClearActive || irBeepingActive) resetIRBeep(); }

  if (!cardPresent && (now - lastRFIDReinit >= RFID_REINIT_MS)) {
    rfid.PCD_Init(); rfid.PCD_SetAntennaGain(rfid.RxGain_max); lastRFIDReinit = now;
  }

  if (rfid.PICC_IsNewCardPresent()) {
    if (!cardPresent && rfid.PICC_ReadCardSerial()) {
      cardPresent = true; resetIRBeep();
      String uid = getUID(); lastUID = uid;
      if (isAuthorized(uid)) {
        lastAccessResult = "GRANTED";
        showGranted(uid); beepGranted(); unlockDoor();
      } else {
        lastAccessResult = "DENIED";
        showDenied(uid); beepDenied();
      }
      rfid.PICC_HaltA(); rfid.PCD_StopCrypto1();
    }
  } else if (cardPresent) { cardPresent = false; }
  delay(10);
}

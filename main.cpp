#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <esp_wifi.h>
#include <math.h>

// --- Define missing structs for packet parsing ---
typedef struct {
  uint16_t frame_ctrl;
  uint8_t duration_id[2];
  uint8_t addr1[6];
  uint8_t addr2[6];
  uint8_t addr3[6];
  uint16_t sequence_ctrl;
  uint8_t addr4[6];
} wifi_ieee80211_mac_hdr_t;

typedef struct {
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0];
} wifi_ieee80211_packet_t;

// Modern web interface with radar visualization
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>ESP32 Wi-Fi Radar</title>
  <style>
    :root {
      --primary: #0f0c29;
      --secondary: #302b63;
      --accent: #00c9ff;
      --text: #e6f1ff;
    }
    
    body {
      background: linear-gradient(135deg, var(--primary), var(--secondary));
      color: var(--text);
      font-family: 'Segoe UI', sans-serif;
      margin: 0;
      padding: 20px;
      height: 100vh;
      overflow: hidden;
    }
    
    .container {
      max-width: 1200px;
      margin: 0 auto;
      height: 100%;
      display: flex;
      flex-direction: column;
    }
    
    header {
      text-align: center;
      padding: 20px 0;
    }
    
    h1 {
      font-size: 2.5rem;
      margin: 0;
      background: linear-gradient(90deg, var(--accent), #92fe9d);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      text-shadow: 0 0 15px rgba(0,201,255,0.3);
    }
    
    .dashboard {
      display: flex;
      flex: 1;
      gap: 30px;
    }
    
    .radar-container {
      flex: 3;
      background: rgba(0,0,0,0.2);
      border-radius: 20px;
      backdrop-filter: blur(10px);
      border: 1px solid rgba(255,255,255,0.1);
      overflow: hidden;
      position: relative;
    }
    
    #radar {
      width: 100%;
      height: 100%;
    }
    
    .device-list {
      flex: 1;
      background: rgba(0,0,0,0.2);
      border-radius: 20px;
      padding: 20px;
      backdrop-filter: blur(10px);
      border: 1px solid rgba(255,255,255,0.1);
      overflow-y: auto;
    }
    
    .device-card {
      background: rgba(255,255,255,0.05);
      border-radius: 10px;
      padding: 15px;
      margin-bottom: 15px;
      animation: fadeIn 0.5s;
    }
    
    @keyframes fadeIn {
      from { opacity: 0; transform: translateY(10px); }
      to { opacity: 1; transform: translateY(0); }
    }
    
    .signal-bar {
      height: 5px;
      background: rgba(255,255,255,0.1);
      border-radius: 3px;
      margin-top: 10px;
      overflow: hidden;
    }
    
    .signal-level {
      height: 100%;
      background: linear-gradient(90deg, #00c9ff, #92fe9d);
      border-radius: 3px;
    }
    
    footer {
      text-align: center;
      padding: 20px;
      font-size: 0.8rem;
      color: rgba(255,255,255,0.5);
    }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <h1>ESP32 Wi-Fi Radar</h1>
      <p>Real-time environment mapping using Wi-Fi signals</p>
    </header>
    
    <div class="dashboard">
      <div class="radar-container">
        <canvas id="radar"></canvas>
      </div>
      
      <div class="device-list" id="deviceList">
        <h2>Detected Devices</h2>
        <div id="devices"></div>
      </div>
    </div>
    
    <footer>
      ESP32 Wi-Fi Sensing Technology | Active Channel: 6
    </footer>
  </div>

  <script>
    const radar = document.getElementById('radar');
    const ctx = radar.getContext('2d');
    const deviceContainer = document.getElementById('devices');
    
    // Setup canvas
    function resizeCanvas() {
      radar.width = radar.offsetWidth;
      radar.height = radar.offsetHeight;
      drawRadarBackground();
    }
    
    window.addEventListener('resize', resizeCanvas);
    resizeCanvas();
    
    // Radar base drawing
    function drawRadarBackground() {
      const width = radar.width;
      const height = radar.height;
      const centerX = width / 2;
      const centerY = height / 2;
      const radius = Math.min(centerX, centerY) * 0.9;
      
      ctx.clearRect(0, 0, width, height);
      
      // Draw concentric circles
      ctx.strokeStyle = 'rgba(0, 201, 255, 0.2)';
      ctx.lineWidth = 1;
      
      for(let i = 1; i <= 5; i++) {
        ctx.beginPath();
        ctx.arc(centerX, centerY, radius * i/5, 0, Math.PI * 2);
        ctx.stroke();
      }
      
      // Draw crosshairs
      ctx.beginPath();
      ctx.moveTo(centerX, 0);
      ctx.lineTo(centerX, height);
      ctx.moveTo(0, centerY);
      ctx.lineTo(width, centerY);
      ctx.stroke();
      
      // Draw sweep line
      const sweepAngle = (Date.now() / 30) % 360;
      ctx.beginPath();
      ctx.moveTo(centerX, centerY);
      ctx.lineTo(
        centerX + radius * Math.cos((sweepAngle - 90) * Math.PI / 180),
        centerY + radius * Math.sin((sweepAngle - 90) * Math.PI / 180)
      );
      ctx.strokeStyle = 'rgba(0, 255, 100, 0.7)';
      ctx.lineWidth = 2;
      ctx.stroke();
    }
    
    // Draw detected devices
    function drawDevices(devices) {
      const width = radar.width;
      const height = radar.height;
      const centerX = width / 2;
      const centerY = height / 2;
      const radius = Math.min(centerX, centerY) * 0.9;
      
      // Clear previous dots but keep background
      ctx.clearRect(0, 0, width, height);
      drawRadarBackground();
      
      devices.forEach(device => {
        const angle = device.angle * Math.PI / 180;
        const distance = radius * (device.distance / 10);
        
        const x = centerX + distance * Math.cos(angle - Math.PI/2);
        const y = centerY + distance * Math.sin(angle - Math.PI/2);
        
        // Draw device dot
        ctx.beginPath();
        ctx.arc(x, y, 8, 0, Math.PI * 2);
        ctx.fillStyle = `rgba(146, 254, 157, ${device.strength/100})`;
        ctx.fill();
        
        // Draw pulse effect
        ctx.beginPath();
        ctx.arc(x, y, 15 * (1 + Math.sin(Date.now()/200)/2), 0, Math.PI * 2);
        ctx.strokeStyle = `rgba(0, 201, 255, 0.3)`;
        ctx.lineWidth = 2;
        ctx.stroke();
      });
    }
    
    // Update device list
    function updateDeviceList(devices) {
      deviceContainer.innerHTML = '';
      
      if(devices.length === 0) {
        deviceContainer.innerHTML = '<div class="device-card">No devices detected</div>';
        return;
      }
      
      devices.forEach(device => {
        const deviceElement = document.createElement('div');
        deviceElement.className = 'device-card';
        deviceElement.innerHTML = `
          <h3>${device.mac}</h3>
          <p>Distance: ${device.distance.toFixed(1)}m | Angle: ${device.angle}°</p>
          <div class="signal-bar">
            <div class="signal-level" style="width: ${device.strength}%"></div>
          </div>
        `;
        deviceContainer.appendChild(deviceElement);
      });
    }
    
    // WebSocket connection
    const ws = new WebSocket('ws://' + window.location.hostname + '/ws');
    
    ws.onmessage = function(event) {
      const data = JSON.parse(event.data);
      drawDevices(data.devices);
      updateDeviceList(data.devices);
    };
    
    // Animation loop
    function animate() {
      drawRadarBackground();
      requestAnimationFrame(animate);
    }
    
    animate();
  </script>
</body>
</html>
)rawliteral";

// Settings
const char* ssid = "ESP32-WiFi-Radar";
const char* password = "radar12345";
const int channel = 6;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Wi-Fi device storage
#define MAX_DEVICES 20
struct WiFiDevice {
  uint8_t mac[6];
  int8_t rssi;
  unsigned long lastSeen;
};
WiFiDevice devices[MAX_DEVICES];
int deviceCount = 0;

// Radar visualization settings
const int MAX_DISTANCE = 10; // meters

void initWiFi() {
  WiFi.softAP(ssid, password, channel);
  Serial.println("Access Point Started");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Configure for promiscuous mode
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_max_tx_power(84); // Increase power to 20dBm (max)
}

void addOrUpdateDevice(uint8_t* mac, int8_t rssi) {
  for (int i = 0; i < deviceCount; i++) {
    if (memcmp(devices[i].mac, mac, 6) == 0) {
      devices[i].rssi = rssi;
      devices[i].lastSeen = millis();
      return;
    }
  }
  if (deviceCount < MAX_DEVICES) {
    memcpy(devices[deviceCount].mac, mac, 6);
    devices[deviceCount].rssi = rssi;
    devices[deviceCount].lastSeen = millis();
    deviceCount++;
  }
}

uint16_t hashMacToAngle(const uint8_t* mac) {
  uint16_t sum = 0;
  for (int i = 0; i < 6; i++) {
    sum += mac[i];
  }
  return sum % 360;
}

String generateDeviceJSON() {
  DynamicJsonDocument doc(1024);
  JsonArray devicesArray = doc.createNestedArray("devices");

  for (int i = 0; i < deviceCount; i++) {
    if (millis() - devices[i].lastSeen < 10000) { // Only recent devices
      JsonObject device = devicesArray.createNestedObject();

      char macStr[18];
      snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
               devices[i].mac[0], devices[i].mac[1], devices[i].mac[2],
               devices[i].mac[3], devices[i].mac[4], devices[i].mac[5]);
      device["mac"] = macStr;

      // Distance estimation from RSSI (real signal strength)
      float distance = exp((float)(-devices[i].rssi - 45) / 20.0);
      if (distance > MAX_DISTANCE) distance = MAX_DISTANCE;
      device["distance"] = distance;

      // Angle derived from MAC hash (stable angle per device)
      uint16_t angle = hashMacToAngle(devices[i].mac);
      device["angle"] = angle;

      // Signal strength mapped 0-100
      device["strength"] = constrain(map(devices[i].rssi, -95, -35, 0, 100), 0, 100);
    }
  }

  String output;
  serializeJson(doc, output);
  return output;
}

void sendRadarData() {
  String json = generateDeviceJSON();
  ws.textAll(json);
}

void processWiFiPackets() {
  esp_wifi_set_promiscuous_rx_cb([](void *buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT && type != WIFI_PKT_DATA && type != WIFI_PKT_MISC) {
      return;
    }

    const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
    const wifi_ieee80211_packet_t *packet = (wifi_ieee80211_packet_t *)ppkt->payload;

    uint8_t *mac = (uint8_t *)packet->hdr.addr2;

    int8_t rssi = ppkt->rx_ctrl.rssi;

    addOrUpdateDevice(mac, rssi);
  });
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
               void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("WebSocket client #%u connected\n", client->id());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  initWiFi();

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", HTML_PAGE);

  });

  server.begin();

  processWiFiPackets();

  Serial.println("Setup done.");
}

void loop() {
  sendRadarData();
  delay(1000);  // Send updates every 1 second
}

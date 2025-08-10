Wifi Radar Scanner for ESP32

An ESP32-based Wi-Fi radar scanner that detects nearby devices, estimates their distance, assigns a stable angle for visualization, and displays them in a real-time, interactive radar interface over Wi-Fi.

The device operates in promiscuous mode to capture Wi-Fi packets, allowing you to visualize device locations without connecting to them.

It features a modern web-based UI with a live radar sweep, device list, and signal strength indicators.


📋 TABLE OF CONTENET

-Introduction

-Features

-Hardware Requirements

-Installation

-Configuration

-Usage

-How It Works

-Dependencies

-Troubleshooting


📖 INTRODUCTION

The Wifi Radar Scanner for ESP32 turns your ESP32 into a standalone device that acts as a radar for Wi-Fi devices. It broadcasts its own Wi-Fi network, serves a real-time visualization webpage, and scans for nearby MAC addresses. Distances are estimated based on RSSI values, while angles are pseudo-randomized but stable per device for visual placement.

This can be used for:

Network presence monitoring

Demonstrating Wi-Fi sensing technology

Building interactive IoT visualization tools



✨ FEATURES

📡 Real-time Wi-Fi device detection using ESP32 promiscuous mode.

🎯 Distance estimation based on RSSI signal strength.

🧭 Stable angle mapping for consistent radar positioning.


🌐 Modern web interface with:

Live radar sweep animation.

Device list with MAC addresses, distances, angles, and signal bars.

🔌 No PC required — the ESP32 serves the webpage itself.


📶 Configurable Wi-Fi AP mode.

🛠 Hardware Requirements

ESP32 development board (with Wi-Fi capability)

USB cable for programming

Computer with PlatformIO or Arduino IDE


📦 INSTALLATION

Clone the repository

```
git clone https://github.com/Meliodus254/Wifi-Radar-Scanner-for-ESP32.git
cd Wifi-Radar-Scanner-for-ESP32
```
Open in PlatformIO (VS Code)

Ensure platformio.ini is configured for your ESP32 board.

Upload to ESP32

Connect your ESP32 and run:

```
pio run --target upload
```


⚙️ CONFIGURATION

You can edit these values in main.cpp:

```
const char* ssid = "ESP32-WiFi-Radar";  // Wi-Fi network name
const char* password = "radar12345";    // Wi-Fi password
const int channel = 6;                  // Wi-Fi channel to scan on
```


🚀 USAGE

Power on the ESP32 after flashing.

Connect to the ESP32’s Wi-Fi network (default: ESP32-WiFi-Radar / password: radar12345).

Open a browser and navigate to:

```
http://192.168.4.1
```

The radar UI will load and start displaying detected devices in real-time.


🧩 HOW IT WORKS

Packet Capture

ESP32 runs in promiscuous mode to receive nearby Wi-Fi frames.

Device Tracking

MAC addresses are stored, RSSI is updated, and inactive devices are removed after 10 seconds.

Distance Estimation

Based on RSSI formula:

```
distance = exp((-RSSI - 45) / 20)
Capped at MAX_DISTANCE.
```


Angle Assignment

Hash of the MAC address ensures stable angle placement on the radar.

Web Interface

Uses WebSockets for real-time updates from ESP32 to the browser.


📚 DEPENDANCIES

Defined in platformio.ini:

WiFi.h – ESP32 Wi-Fi control.

AsyncTCP – Async TCP server.

ESPAsyncWebServer – Non-blocking web server.

ArduinoJson – JSON serialization for device data.

esp_wifi.h – Low-level Wi-Fi driver functions.

🛠 TROUBLESHOOTING

No devices detected?

Ensure your ESP32 supports promiscuous mode (all standard models do).

Try a different Wi-Fi channel.

Radar page not loading?

Clear browser cache.

Check that you are connected to the ESP32’s AP network.

Inaccurate distances?

RSSI-based distance is inherently approximate; environmental factors can affect readings.

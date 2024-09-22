#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <RF24.h>
#include <ArduinoJson.h>

// Wi-Fi credentials
const char* ssid = "EvilCrowRF";
const char* password = "12345678";

// NRF24L01 setup: CE -> D1, CSN -> D2
RF24 radio(D1, D2);

// Web server setup
ESP8266WebServer server(80);

// Variables for scanning and jamming
bool isJamming = false;
bool isScanning = false;
uint8_t detectedDevices[32];  // Array to store detected devices on each channel

void setup() {
  Serial.begin(115200);

  // Start NRF24L01
  if (!radio.begin()) {
    Serial.println("NRF24L01 initialization failed!");
  } else {
    Serial.println("NRF24L01 initialized.");
  }

  // Set up radio parameters
  radio.setPALevel(RF24_PA_LOW);  // Low power
  radio.setDataRate(RF24_1MBPS);  // Standard data rate

  // Start Wi-Fi Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Created");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/status", handleStatus);  // Status of scanning and jamming
  server.on("/start_jamming", handleStartJamming);
  server.on("/stop_jamming", handleStopJamming);
  server.on("/start_scanning", handleStartScanning);
  server.on("/stop_scanning", handleStopScanning);

  // Start the web server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
  if (isScanning) {
    scanDevices();  // Perform the device scanning in real-time
  }
  if (isJamming) {
    jamChannel();  // Jam the channel in real-time
  }
}

// Serve the web page
void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
  <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>EvilCrow RF Control</title>
      <style>
          body {
              font-family: Arial, sans-serif;
              text-align: center;
              background-color: #1e1e1e;
              color: white;
              margin: 0;
              padding: 0;
          }
          header {
              padding: 10px;
              background-color: #333;
              color: #f1f1f1;
          }
          h1 {
              margin: 0;
              padding: 20px;
          }
          .container {
              padding: 30px;
          }
          .btn {
              padding: 15px 30px;
              font-size: 18px;
              margin: 10px;
              color: white;
              background-color: #555;
              border: none;
              border-radius: 5px;
              cursor: pointer;
              transition: background-color 0.3s;
          }
          .btn:hover {
              background-color: #f44336;
          }
          .footer {
              margin-top: 50px;
              padding: 20px;
              background-color: #333;
              color: #f1f1f1;
          }
      </style>
      <script>
        function updateStatus() {
          fetch("/status")
            .then(response => response.json())
            .then(data => {
              document.getElementById("status").innerHTML = "Status: " + (data.jamming ? "Jamming" : (data.scanning ? "Scanning" : "Idle"));
              document.getElementById("devices").innerHTML = "Detected Devices: " + data.devices;
            });
        }
        setInterval(updateStatus, 2000);  // Real-time updates every 2 seconds
      </script>
  </head>
  <body onload="updateStatus()">
      <header>
          <h1>EvilCrow RF Control Panel</h1>
      </header>
      <div class="container">
          <p id="status">Status: Idle</p>
          <p id="devices">Detected Devices: None</p>
          <button class="btn" onclick="window.location.href='/start_jamming'">Start Jamming</button><br>
          <button class="btn" onclick="window.location.href='/stop_jamming'">Stop Jamming</button><br>
          <button class="btn" onclick="window.location.href='/start_scanning'">Start Scanning</button><br>
          <button class="btn" onclick="window.location.href='/stop_scanning'">Stop Scanning</button><br>
      </div>
      <div class="footer">
          <p>&copy; 2024 EvilCrow RF</p>
      </div>
  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

// Function to provide real-time status
void handleStatus() {
  StaticJsonDocument<200> doc;
  doc["jamming"] = isJamming;
  doc["scanning"] = isScanning;
  String detectedDevicesStr = "";
  for (int i = 0; i < 32; i++) {
    if (detectedDevices[i] > 0) {
      detectedDevicesStr += "Channel " + String(i + 1) + ": " + String(detectedDevices[i]) + " devices<br>";
    }
  }
  doc["devices"] = detectedDevicesStr;

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

// Function to start jamming
void handleStartJamming() {
  isJamming = true;
  server.send(200, "text/plain", "Jamming Started");
  Serial.println("Jamming Started");
}

// Function to stop jamming
void handleStopJamming() {
  isJamming = false;
  server.send(200, "text/plain", "Jamming Stopped");
  Serial.println("Jamming Stopped");
}

// Function to start scanning for devices
void handleStartScanning() {
  isScanning = true;
  server.send(200, "text/plain", "Scanning Started");
  Serial.println("Scanning Started");
}

// Function to stop scanning for devices
void handleStopScanning() {
  isScanning = false;
  server.send(200, "text/plain", "Scanning Stopped");
  Serial.println("Scanning Stopped");
}

// Function to scan devices on each channel
void scanDevices() {
  for (int i = 0; i < 32; i++) {
    radio.setChannel(i);  // Set the radio to each channel
    delay(10);  // Wait for traffic
    if (radio.available()) {
      detectedDevices[i]++;  // Increment the device count for that channel
    }
  }
}

// Function to jam the currently set channel
void jamChannel() {
  for (int i = 0; i < 32; i++) {
    radio.setChannel(i);  // Set to the next channel
    radio.stopListening();  // Stop listening to transmit noise
    radio.write("noise", sizeof("noise"));  // Send out noise
    delay(5);  // Keep moving to prevent being detected
  }
}

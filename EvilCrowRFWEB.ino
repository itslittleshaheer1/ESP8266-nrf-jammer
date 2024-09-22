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

// Jamming parameters
unsigned long jammingDuration = 10000;  // Jamming duration in milliseconds
unsigned long jammingInterval = 5000;   // Interval between jamming sessions
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);

  // Start NRF24L01
  if (!radio.begin()) {
    Serial.println("NRF24L01 initialization failed!");
  } else {
    Serial.println("NRF24L01 initialized.");
  }

  // Set up radio parameters for jamming
  radio.setAutoAck(false);  // Disable acknowledgments
  radio.stopListening();    // Stop listening for responses
  radio.setRetries(0, 0);   // No retries
  radio.setPayloadSize(1);  // Minimal payload size
  radio.setAddressWidth(3); // Set address width
  radio.setPALevel(RF24_PA_MAX); // Set max power level for jamming
  radio.setDataRate(RF24_2MBPS); // High data rate for effective jamming
  radio.setCRCLength(RF24_CRC_DISABLED); // Disable CRC for raw transmission

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
  unsigned long currentMillis = millis();

  // Manage jamming intervals
  if (currentMillis - previousMillis >= jammingInterval) {
    previousMillis = currentMillis;
    if (isJamming) {
      Serial.println("Jamming Started");
      jamChannel();  // Start jamming
      delay(jammingDuration);  // Jam for a set duration
    }
  }

  if (isScanning) {
    scanDevices();  // Perform the device scanning in real-time
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
              background-color: #1e1e1e;
              color: white;
              margin: 0;
              padding: 0;
              display: flex;
          }
          header {
              padding: 10px;
              background-color: #333;
              color: #f1f1f1;
              width: 100%;
              text-align: center;
          }
          h1 {
              margin: 0;
              padding: 20px;
          }
          .container {
              padding: 30px;
              width: 70%;
          }
          .sidebar {
              padding: 30px;
              width: 30%;
              background-color: #333;
              height: 100vh;
              overflow-y: auto;
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
          .device-list {
              list-style-type: none;
              padding: 0;
          }
          .device-list li {
              margin-bottom: 10px;
              font-size: 18px;
          }
      </style>
      <script>
        function updateStatus() {
          fetch("/status")
            .then(response => response.json())
            .then(data => {
              document.getElementById("status").innerHTML = "Status: " + (data.jamming ? "Jamming" : (data.scanning ? "Scanning" : "Idle"));
              document.getElementById("devices").innerHTML = data.devices;
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
          <button class="btn" onclick="window.location.href='/start_jamming'">Start Jamming</button><br>
          <button class="btn" onclick="window.location.href='/stop_jamming'">Stop Jamming</button><br>
          <button class="btn" onclick="window.location.href='/start_scanning'">Start Scanning</button><br>
          <button class="btn" onclick="window.location.href='/stop_scanning'">Stop Scanning</button><br>
      </div>
      <div class="sidebar">
          <h2>Detected Devices</h2>
          <ul id="devices" class="device-list">
              <li>No devices detected</li>
          </ul>
      </div>
  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

// Function to provide real-time status and devices list
void handleStatus() {
  StaticJsonDocument<500> doc;
  doc["jamming"] = isJamming;
  doc["scanning"] = isScanning;
  String devicesList = "<ul class='device-list'>";
  for (int i = 0; i < 32; i++) {
    if (detectedDevices[i] > 0) {
      devicesList += "<li>Channel " + String(i) + ": " + String(detectedDevices[i]) + " devices</li>";
    }
  }
  devicesList += "</ul>";
  doc["devices"] = devicesList;

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
  memset(detectedDevices, 0, sizeof(detectedDevices));  // Clear the detected devices array
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
    radio.write("JAMMING_PAYLOAD", sizeof("JAMMING_PAYLOAD"));  // Send out noise
    delay(random(5, 20));  // Random delay
    radio.write("", 0);  // Send silence
    delay(random(5, 20));  // Random delay before next transmission
  }
}

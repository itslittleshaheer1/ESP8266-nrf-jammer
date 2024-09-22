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

// Variables
bool isJamming = false;
bool isScanning = false;
char capturedSignal[32] = {0};  // Store captured signal
String capturedSignalHex = "";

// Status Update Struct
struct {
  bool jamming;
  bool scanning;
  String signal;
} status = {false, false, ""};

void setup() {
  Serial.begin(115200);

  // Start NRF24L01
  if (!radio.begin()) {
    Serial.println("NRF24L01 initialization failed!");
  } else {
    Serial.println("NRF24L01 initialized.");
  }

  // Configure NRF24L01 for optimized power and range
  radio.setPALevel(RF24_PA_LOW);  // Low power
  radio.setDataRate(RF24_250KBPS);  // Extended range

  // Start Wi-Fi Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Created");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/status", handleStatus);  // New route for status updates
  server.on("/start_jamming", handleStartJamming);
  server.on("/stop_jamming", handleStopJamming);
  server.on("/start_scanning", handleStartScanning);
  server.on("/stop_scanning", handleStopScanning);
  server.on("/replay_signal", handleReplaySignal);

  // Start the web server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}

// Function to serve the web interface
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
              document.getElementById("signal").innerHTML = "Captured Signal: " + data.signal;
            });
        }
        setInterval(updateStatus, 2000);
      </script>
  </head>
  <body onload="updateStatus()">
      <header>
          <h1>EvilCrow RF Control Panel</h1>
      </header>
      <div class="container">
          <p id="status">Status: Idle</p>
          <p id="signal">Captured Signal: None</p>
          <button class="btn" onclick="window.location.href='/start_jamming'">Start Jamming</button><br>
          <button class="btn" onclick="window.location.href='/stop_jamming'">Stop Jamming</button><br>
          <button class="btn" onclick="window.location.href='/start_scanning'">Start Scanning</button><br>
          <button class="btn" onclick="window.location.href='/stop_scanning'">Stop Scanning</button><br>
          <button class="btn" onclick="window.location.href='/replay_signal'">Replay Signal</button><br>
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
  doc["signal"] = capturedSignalHex;
  
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

// Function to capture and convert signal to hex
void captureSignal() {
  if (radio.available()) {
    radio.read(&capturedSignal, sizeof(capturedSignal));
    
    // Convert captured signal to hex string
    capturedSignalHex = "";
    for (int i = 0; i < sizeof(capturedSignal); i++) {
      char hex[3]; // 2 characters for hex + 1 for null terminator
      sprintf(hex, "%02X", capturedSignal[i]);
      capturedSignalHex += hex;
    }
    
    Serial.println("Signal Captured: " + capturedSignalHex);
  }
}

// Start jamming function
void handleStartJamming() {
  isJamming = true;
  server.send(200, "text/plain", "Jamming Started");
  Serial.println("Jamming Started");
}

// Stop jamming function
void handleStopJamming() {
  isJamming = false;
  server.send(200, "text/plain", "Jamming Stopped");
  Serial.println("Jamming Stopped");
}

// Start scanning function
void handleStartScanning() {
  isScanning = true;
  server.send(200, "text/plain", "Scanning Started");
  Serial.println("Scanning Started");

  while (isScanning) {
    captureSignal(); // Capture signals
  }
}

// Stop scanning function
void handleStopScanning() {
  isScanning = false;
  server.send(200, "text/plain", "Scanning Stopped");
  Serial.println("Scanning Stopped");
}

// Function to replay captured signal
void handleReplaySignal() {
  if (capturedSignalHex.length() > 0) {
    radio.write(&capturedSignal, sizeof(capturedSignal));
    server.send(200, "text/plain", "Signal Replayed");
    Serial.println("Signal Replayed: " + capturedSignalHex);
  } else {
    server.send(200, "text/plain", "No Signal to Replay");
  }
}

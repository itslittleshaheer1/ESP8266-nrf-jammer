#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE  1
RF24 radio(1, 2);

// Replace with your desired network credentials
const char* ssid = "ESP8266_Jammer";   // SSID of the AP
const char* password = "password";      // Password for the AP

ESP8266WebServer server(80);
bool jamming = false;

void setup() {
  Serial.begin(57600);
  
  // Start the WiFi in Access Point mode
  WiFi.softAP(ssid, password);
  Serial.println("WiFi AP started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Start web server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/start", HTTP_GET, startJamming);
  server.on("/stop", HTTP_GET, stopJamming);
  server.begin();
  Serial.println("HTTP server started");

  // NRF24L01 setup
  radio.begin();
  radio.startListening();
  radio.stopListening();

  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setBitOrder(MSBFIRST);
  
  pinMode(CE, OUTPUT);
  disable();
  powerUp();

  setRegister(_NRF24_EN_AA, 0x0);
  setRegister(_NRF24_RF_SETUP, 0x0F);
  
  Serial.println("NRF24L01 Initialized");
}

void loop() {
  server.handleClient();
  
  if (jamming) {
    jammer();
    delay(100);
  }
}

void handleRoot() {
  String html = "<html><body><h1>NRF24 Jammer Control</h1>"
                "<a href=\"/start\">Start Jamming</a><br>"
                "<a href=\"/stop\">Stop Jamming</a>"
                "</body></html>";
  server.send(200, "text/html", html);
}

void startJamming() {
  jamming = true;
  server.send(200, "text/html", "Jamming started. <a href=\"/\">Go back</a>");
  Serial.println("Jamming started");
}

void stopJamming() {
  jamming = false;
  server.send(200, "text/html", "Jamming stopped. <a href=\"/\">Go back</a>");
  Serial.println("Jamming stopped");
}

void setRegister(byte r, byte v) {
  PORTB &= ~_BV(2);
  SPI.transfer((r & 0x1F) | 0x20);
  SPI.transfer(v);
  PORTB |= _BV(2);
}

void powerUp(void) {
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) | 0x02);
  delayMicroseconds(130);
}

void powerDown(void) {
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) & ~0x02);
}

void enable(void) {
  PORTB |= _BV(1);
}

void disable(void) {
  PORTB &= ~_BV(1);
}

void setRX(void) {
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) | 0x01);
  enable();
  delayMicroseconds(100);
}

void jammer() {
  const char text[] = "xxxxxxxxxxxxxxxx"; // Send the noise
  for (int i = 0; i < 23; i++) { // Adjust based on your needs
    radio.setChannel(i);
    radio.write(&text, sizeof(text));
  }
}

byte getRegister(byte r) {
  PORTB &= ~_BV(2);
  SPI.transfer(r & 0x1F);
  byte result = SPI.transfer(0);
  PORTB |= _BV(2);
  return result;
}

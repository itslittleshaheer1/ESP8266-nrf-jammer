#include <SPI.h>
#include <RF24.h>

// Define NRF24L01 connections with the new pinout
#define CE_PIN 1   // D1
#define CSN_PIN 2  // D2
#define SCK_PIN 5  // D5
#define MOSI_PIN 7  // D7
#define MISO_PIN 6  // D6
#define IRQ_PIN 4   // D4

// Create an instance of the RF24 class
RF24 radio(CE_PIN, CSN_PIN);

void setup() {
  Serial.begin(115200);
  
  // Initialize the NRF24L01 module
  radio.begin();
  radio.setChannel(76); // Set to a common channel (adjust as needed)
  radio.setPayloadSize(32); // Set payload size
  radio.stopListening(); // Stop listening to allow sending

  Serial.println("Jammer initialized. Starting jamming...");
}

void loop() {
  uint8_t jamData[32] = {0}; // Jam signal
  
  // Fill jamData with random data for jamming
  for (int i = 0; i < sizeof(jamData); i++) {
    jamData[i] = random(0, 256); // Generate random bytes
  }

  // Continuously send jam signals
  radio.write(&jamData, sizeof(jamData));
  Serial.println("Sending Jam Signal...");

  delay(10); // Adjust jamming frequency as needed
}

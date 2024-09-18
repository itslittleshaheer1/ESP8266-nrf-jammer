#include <SPI.h>
#include <RF24.h>

// Define nRF24L01 pinout
#define CE_PIN   5   // D1 (GPIO5)
#define CSN_PIN  4   // D2 (GPIO4)

// Create RF24 object
RF24 radio(CE_PIN, CSN_PIN);

// Set the jamming channel (adjust as needed, 0-125)
byte channel = 0; // Jam the lowest 2.4GHz channel for maximum coverage

void setup() {
  Serial.begin(115200);

  // Initialize the RF24 radio
  if (radio.begin()) {
    Serial.println("Jammer is Started !!!");
    radio.setAutoAck(false);    // Disable acknowledgments
    radio.stopListening();      // Stop listening for incoming data
    radio.setRetries(0, 0);     // No retries
    radio.setPayloadSize(1);    // Minimal payload size
    radio.setAddressWidth(3);   // Set address width
    radio.setPALevel(RF24_PA_MAX);  // Set max power level for jamming
    radio.setDataRate(RF24_2MBPS);  // High data rate
    radio.setCRCLength(RF24_CRC_DISABLED);  // Disable CRC
    radio.setChannel(channel);  // Set the jamming channel
    radio.startConstCarrier(RF24_PA_MAX, 0); // Start continuous carrier signal
  } else {
    Serial.println("Jammer couldn't be started !!!");
  }
}

void loop() {
  // Keep the jamming continuous
  yield();  // Prevent watchdog reset on ESP8266
}

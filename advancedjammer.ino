#include <SPI.h>
#include <RF24.h>

// Define the CE and CSN pins for nRF24L01 (based on your connections)
#define CE_PIN 5   // D1 (GPIO5)
#define CSN_PIN 4  // D2 (GPIO4)

// Create RF24 object
RF24 radio(CE_PIN, CSN_PIN);

// Set the channel you want to jam (0-125, where each channel represents a frequency in the 2.4 GHz band)
int channel = 108; // Example: Channel 108 (2.508 GHz), change as needed

void setup() {
  Serial.begin(115200);
  radio.begin();
  radio.setChannel(channel);       // Set the desired channel for jamming
  radio.setPALevel(RF24_PA_HIGH);  // Set power level to maximum for more signal strength
  radio.setDataRate(RF24_2MBPS);   // Use a high data rate to maximize interference
  radio.setCRCLength(RF24_CRC_8);  // Set the CRC length to 8 bits (optional)
  radio.openWritingPipe(0xE8E8F0F0E1LL);  // Set an arbitrary address for transmission
}

void loop() {
  // Send random noise or data to jam the selected channel
  const char data[] = "JAM";
  radio.write(&data, sizeof(data));  // Transmit "JAM" data

  // Optional: Print status for debugging
  Serial.println("Jamming channel " + String(channel) + "...");
  delay(10);  // Short delay to keep the jamming continuous
}

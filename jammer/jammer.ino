#include <RF24.h>
#include <ESP8266WiFi.h> // Ensure the ESP8266 WiFi library is included


// Define pin mappings based on the pinout provided
#define CE_PIN   D1  // CE to GPIO5
#define CSN_PIN  D2  // CSN to GPIO4


// Initialize RF24 with CE and CSN pins
RF24 radio(CE_PIN, CSN_PIN);


// Channel range (0 to 79 is typical for the NRF24L01)
const byte minChannel = 0;
const byte maxChannel = 79;


void setup(void) {
  // Disable WiFi
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin(); // Ensures WiFi is completely off


  Serial.begin(115200);
  initRadio();
}


void initRadio() {
  if (radio.begin()) {  // Use default SPI interface
    delay(200);
    Serial.println("Jamming Started !!!");


    radio.setAutoAck(false);   // Disable auto acknowledgment
    radio.stopListening();     // Set to transmit mode
    radio.setRetries(0, 0);    // No retries
    radio.setPayloadSize(5);   // Set payload size
    radio.setAddressWidth(3);  // Set address width
    radio.setPALevel(RF24_PA_MAX, true);  // Max power level
    radio.setDataRate(RF24_2MBPS);         // Set data rate
    radio.setCRCLength(RF24_CRC_DISABLED); // Disable CRC


    // Continuous transmission
    while (true) {
      for (byte channel = minChannel; channel <= maxChannel; ++channel) {
        radio.setChannel(channel); // Switch to the current channel
        radio.startConstCarrier(RF24_PA_MAX, 0); // Start constant carrier on the current channel
        delay(100); // Delay to control the frequency of channel switching
        radio.stopListening(); // Ensure we are not in listening mode
      }
    }
  } else {
    Serial.println("Jamming couldn't be started !!!");
  }
}


void loop(void) {
  // Empty loop because the jamming runs in setup
}

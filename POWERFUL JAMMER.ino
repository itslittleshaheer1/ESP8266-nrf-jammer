#include "RF24.h"
#include "nRF24l01.h"

// Define RF24 object with your pinout (CE -> D1, CSN -> D2)
RF24 radio(D1, D2);  // CE pin -> D1, CSN pin -> D2

byte i = 37;  // Start channel (start from 37 for wider coverage)
unsigned int flag = 0;

void initSP() {
  // Initialize SPI (default pins for ESP8266)
  SPI.begin();  // No parameters since default pins are used

  if (radio.begin()) {
    delay(200);
    Serial.println("RF24 Started !!!");
    
    // Configure radio settings
    radio.setAutoAck(false);           // Disable auto-acknowledgment
    radio.stopListening();             // Stop receiving mode, enter transmission mode
    radio.setRetries(0, 0);            // No retries
    radio.setPayloadSize(5);           // Set 5 bytes payload
    radio.setAddressWidth(3);          // Set 3-byte address width
    radio.setPALevel(RF24_PA_MAX);     // Set Power Amplification to maximum
    radio.setDataRate(RF24_2MBPS);     // Set data rate to 2 Mbps (high-speed mode)
    radio.setCRCLength(RF24_CRC_DISABLED);  // Disable CRC for faster performance
    
    // Display radio configuration details
    radio.printPrettyDetails();
    
    // Start transmitting constant carrier on the initial channel
    radio.startConstCarrier(RF24_PA_MAX, i);  
  } else {
    Serial.println("RF24 initialization failed!");
  }
}

void two() {
  // Implement channel hopping with 1-channel spacing for more aggressive jamming
  if (flag == 0) {
    i += 1;  // Change to next channel
  } else {
    i -= 1;  // Change to previous channel
  }

  // Handle channel boundaries (37 to 79)
  if ((i > 79) && (flag == 0)) {
    flag = 1;  // Switch direction
  } else if ((i < 37) && (flag == 1)) {
    flag = 0;  // Switch direction
  }

  // Set the radio to the new channel
  radio.setChannel(i);
}

void setup(void) {
  Serial.begin(115200);  // Start serial for debugging
  initSP();  // Initialize SPI and RF24
}

void loop(void) {
  // Continuously perform channel hopping
  two();  // Call the channel hopping function
  delay(100);  // Small delay for stability; adjust for jamming intensity
}
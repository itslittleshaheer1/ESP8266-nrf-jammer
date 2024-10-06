#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ESP8266WiFi.h>

// Define the ESP8266-specific SPI pins
#define SCK_PIN D5
#define MOSI_PIN D7
#define MISO_PIN D6

// Define the CE and CSN pins for the radios
RF24 radio(D1, D2); // CE, CSN for radio
RF24 radio2(D1, D2); // CE, CSN for radio2
RF24 radio3(D1, D2); // CE, CSN for radio3

void setup() {
  // Initialize the ESP8266-specific SPI pins
  pinMode(SCK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);

  // Initialize the radios
  radio.begin();
  radio2.begin();
  radio3.begin();

  // Power down the radios initially
  radio.powerDown();
  radio2.powerDown();
  radio3.powerDown();
  delay(1000);

  // Setup radio 1
  radio.powerUp();
  radio.setAutoAck(false);  // Very important setting
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_2MBPS);
  radio.stopListening();
  radio.setChannel(80);
  delay(1000);

  // Setup radio 2
  radio2.powerUp();
  radio2.setAutoAck(false);
  radio2.setPALevel(RF24_PA_HIGH);
  radio2.setDataRate(RF24_2MBPS);
  radio2.stopListening();
  radio2.setChannel(26);
  delay(1000);

  // Setup radio 3
  radio3.powerUp();
  radio3.setAutoAck(false);
  radio3.setPALevel(RF24_PA_HIGH);
  radio3.setDataRate(RF24_2MBPS);
  radio3.stopListening();
  radio3.setChannel(2);
  delay(1000);
}

void loop() {
  byte text = 255; // just some random string
  radio.writeFast(&text, sizeof(text));
  radio3.writeFast(&text, sizeof(text));
  radio2.writeFast(&text, sizeof(text));
}

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE  1
RF24 radio(1, 2);

void setup() {
  Serial.begin(57600);
  
  // NRF24L01 setup
  radio.begin();
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
  jammer();
  delay(100); // Adjust delay as needed
}

void setRegister(byte r, byte v) {
  digitalWrite(CE, LOW);
  SPI.transfer((r & 0x1F) | 0x20);
  SPI.transfer(v);
  digitalWrite(CE, HIGH);
}

void powerUp() {
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) | 0x02);
  delayMicroseconds(130);
}

void disable() {
  digitalWrite(CE, LOW);
}

void jammer() {
  const char text[] = "xxxxxxxxxxxxxxxx"; // Send the noise
  for (int i = 0; i < 23; i++) { // Adjust based on your needs
    radio.setChannel(i);
    radio.write(&text, sizeof(text));
  }
}

byte getRegister(byte r) {
  digitalWrite(CE, LOW);
  SPI.transfer(r & 0x1F);
  byte result = SPI.transfer(0);
  digitalWrite(CE, HIGH);
  return result;
}

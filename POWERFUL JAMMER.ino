#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE  1
RF24 radio(1, 2);

#define CHANNELS  80 // Cover the entire 2.4GHz band
int channel[CHANNELS];

int line;

const uint8_t num_channels = 80;
int values[num_channels];
int channels = 0;
const byte address[6] = "00001";
const int num_reps = 50;
bool jamming = true;

void setup()
{
  Serial.begin(57600);

  radio.begin();
  radio.startListening();
  radio.stopListening();

  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setBitOrder(MSBFIRST);

  pinMode(CE, OUTPUT);
  disable();

  powerUp();

#define _NRF24_EN_AA 0x01
#define _NRF24_CONFIG 0x00
#define _NRF24_RF_SETUP 0x06
#define _NRF24_RPD 0x09

  Serial.println("Starting Jamming...");
}

void setRegister(byte r, byte v)
{
  digitalWrite(CE, LOW); // Use digitalWrite instead of manipulating PORTB
  SPI.transfer((r & 0x1F) | 0x20);
  SPI.transfer(v);
  digitalWrite(CE, HIGH); // Use digitalWrite instead of manipulating PORTB
}

byte getRegister(byte r) {
  digitalWrite(CE, LOW); // Use digitalWrite instead of manipulating PORTB
  SPI.transfer((r & 0x1F) | 0x00); // Note the 0x00 instead of 0x20
  byte v = SPI.transfer(0x00); // Send a dummy byte to receive the value
  digitalWrite(CE, HIGH); // Use digitalWrite instead of manipulating PORTB
  return v;
}

void powerUp(void)
{
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) | 0x02);
  delayMicroseconds(130);
}

void powerDown(void)
{
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) & ~0x02);
}

void enable(void)
{
  digitalWrite(CE, HIGH); // Use digitalWrite instead of manipulating PORTB
}

void disable(void)
{
  digitalWrite(CE, LOW); // Use digitalWrite instead of manipulating PORTB
}

void setRX(void)
{
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) | 0x01);
  enable();

  delayMicroseconds(100);
}

void jammer() {
  const int channels = 80; // Cover the entire 2.4GHz band
  const int frequency_step = 5; // Use a 5MHz frequency step

  for (int i = 0; i < channels; i++) {
    int frequency = 2400 + (i * frequency_step); // Calculate the frequency
    radio.setChannel(frequency);
    radio.setPALevel(RF24_PA_HIGH); // Set the transmission power to high

    // Generate a pseudorandom noise (PN) sequence
    uint8_t pn_sequence[32];
    for (int j = 0; j < 32; j++) {
      pn_sequence[j] = (uint8_t)random(256);
    }

    // Transmit the PN sequence
    radio.write(pn_sequence, 32);
    delayMicroseconds(10);
  }
}

void loop()
{
  jammer();
  delay(100);
}

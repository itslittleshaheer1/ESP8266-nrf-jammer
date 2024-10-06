#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE  1
RF24 radio(1, 2);

#define CHANNELS  64
int channel[CHANNELS];

int  line;

const uint8_t num_channels = 64;
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

  pinMode(CE,OUTPUT);
  disable();

  powerUp();

  setRegister(_NRF24_EN_AA,0x0);
  setRegister(_NRF24_RF_SETUP,0x0F);

  Serial.println("Starting Jamming...");
}

void setRegister(byte r, byte v)
{
  PORTB &=~_BV(2);
  SPI.transfer((r&0x1F)|0x20);
  SPI.transfer(v);
  PORTB |= _BV(2);
}

void powerUp(void)
{
  setRegister(_NRF24_CONFIG,getRegister(_NRF24_CONFIG)|0x02);
  delayMicroseconds(130);
}

void powerDown(void)
{
  setRegister(_NRF24_CONFIG,getRegister(_NRF24_CONFIG)&~0x02);
}

void enable(void)
{
  PORTB |= _BV(1);
}

void disable(void)
{
  PORTB &=~_BV(1);
}

void setRX(void)
{
  setRegister(_NRF24_CONFIG,getRegister(_NRF24_CONFIG)|0x01);
  enable();

  delayMicroseconds(100);
}

void jammer() {
  const char text[] = "xxxxxxxxxxxxxxxx"; // send the noise
  for (int i = ((channels * 5) + 1); i < ((channels * 5) + 23); i++) {
    radio.setChannel(i);
    radio.write( & text, sizeof(text));
  }
}

void loop()
{
  jammer();
  delay(100);
}

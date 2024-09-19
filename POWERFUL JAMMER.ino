#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>

// Pin mapping for ESP8266
#define CE_PIN   D1  // CE connected to D1
#define CSN_PIN  D2  // CSN connected to D2

// RF24 object with specified pins
RF24 radio(CE_PIN, CSN_PIN);

// Constants
#define CHANNELS  64
int channel[CHANNELS];
char grey[] = " .:-=+*aRW";

// NRF24 registers
#define _NRF24_CONFIG      0x00
#define _NRF24_EN_AA       0x01
#define _NRF24_RF_CH       0x05
#define _NRF24_RF_SETUP    0x06
#define _NRF24_RPD         0x09

// Global variables
int channels = 0;
bool jamming = true;
const byte address[6] = "00001";
const int num_reps = 50;

// Function to get the value from a register
byte getRegister(byte r) {
  byte c;
  digitalWrite(CSN_PIN, LOW);
  c = SPI.transfer(r & 0x1F);
  c = SPI.transfer(0);  
  digitalWrite(CSN_PIN, HIGH);
  return c;
}

// Setup function
void setup() {
  Serial.begin(57600);

  radio.begin();
  radio.startListening();
  radio.stopListening();

  Serial.println("Starting 2.4GHz Scanner ...");
  Serial.println();
  Serial.println("Channel Layout");
  printChannels();

  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setBitOrder(MSBFIRST);

  pinMode(CE_PIN, OUTPUT);
  pinMode(CSN_PIN, OUTPUT);

  disable();
  powerUp();
  setRegister(_NRF24_EN_AA, 0x0); // Disable auto-ack
  setRegister(_NRF24_RF_SETUP, 0x0F); // Set RF setup
}

// Function to set a register value
void setRegister(byte r, byte v) {
  digitalWrite(CSN_PIN, LOW);
  SPI.transfer((r & 0x1F) | 0x20);
  SPI.transfer(v);
  digitalWrite(CSN_PIN, HIGH);
}

// Power up the NRF24L01 module
void powerUp() {
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) | 0x02);
  delayMicroseconds(130);
}

// Power down the NRF24L01 module
void powerDown() {
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) & ~0x02);
}

// Enable the module
void enable() {
  digitalWrite(CE_PIN, HIGH);
}

// Disable the module
void disable() {
  digitalWrite(CE_PIN, LOW);
}

// Set the module to RX mode
void setRX() {
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) | 0x01);
  enable();
  delayMicroseconds(100);
}

// Scan the channels for activity
void scanChannels() {
  disable();
  for (int j = 0; j < 255; j++) {
    for (int i = 0; i < CHANNELS; i++) {
      setRegister(_NRF24_RF_CH, (128 * i) / CHANNELS);
      setRX();
      delayMicroseconds(40);
      disable();
      if (getRegister(_NRF24_RPD) > 0) {
        channel[i]++;
      }
    }
  }
}

// Output the channel activity
void outputChannels() {
  int norm = 0;
  for (int i = 0; i < CHANNELS; i++) {
    if (channel[i] > norm) norm = channel[i];
  }

  Serial.print('|');
  for (int i = 0; i < CHANNELS; i++) {
    int pos = (norm != 0) ? (channel[i] * 10) / norm : 0;
    if (pos == 0 && channel[i] > 0) pos++;
    if (pos > 9) pos = 9;
    Serial.print(grey[pos]);
    channel[i] = 0;
  }
  Serial.print("| ");
  Serial.println(norm);
}

// Print channel layout
void printChannels() {
  Serial.println(">      1 2  3 4  5  6 7 8  9 10 11 12 13  14                     <");
}

// Jamming function
void jammer() {
  const char text[] = "xxxxxxxxxxxxxxxx"; // Noise to send
  for (int i = ((channels * 5) + 1); i < ((channels * 5) + 23); i++) {
    radio.setChannel(i);
    radio.write(&text, sizeof(text));
  }
}

// Main loop
void loop() {
  Serial.print("Scanning channel: ");
  Serial.println(channels + 1);

  if (jamming) {
    Serial.println("Jamming active on channel " + String(channels + 1));
    radio.setPALevel(RF24_PA_HIGH);
    radio.setDataRate(RF24_2MBPS);
  }

  // Call jamming function if jamming is enabled
  if (jamming) {
    jammer();
  }

  // Scan channels and output results
  scanChannels();
  outputChannels();

  // Switch to the next channel
  channels++;
  if (channels > 13) {
    channels = 0; // Reset to first channel after reaching the last
  }

  delay(1000); // Wait 1 second before scanning again
}

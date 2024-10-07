#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>

#define CE_PIN D1 // GPIO 5
#define CSN_PIN D2 // GPIO 4

RF24 radio(CE_PIN, CSN_PIN);

#define CHANNELS  64
int channel[CHANNELS];

char grey[] = " .:-=+*aRW";

// NRF24L01 Register Definitions
#define _NRF24_CONFIG      0x00
#define _NRF24_EN_AA       0x01
#define _NRF24_EN_RXADDR   0x02  // Added for enabling RX addresses
#define _NRF24_SETUP_AW    0x03  // Added for address width setup
#define _NRF24_SETUP_RETR  0x04  // Added for retransmission settings
#define _NRF24_RF_CH       0x05
#define _NRF24_RF_SETUP    0x06
#define _NRF24_STATUS      0x07
#define _NRF24_OBSERVE_TX  0x08
#define _NRF24_RPD         0x09  // Receive Power Detector
#define _NRF24_RTX_ADDR    0x0A
#define _NRF24_RX_ADDR_P0  0x0B
#define _NRF24_RX_ADDR_P1  0x0C
#define _NRF24_RX_ADDR_P2  0x0D
#define _NRF24_RX_ADDR_P3  0x0E
#define _NRF24_RX_ADDR_P4  0x0F
#define _NRF24_RX_ADDR_P5  0x10
#define _NRF24_TX_ADDR     0x10
#define _NRF24_RX_PW_P0    0x11
#define _NRF24_RX_PW_P1    0x12
#define _NRF24_RX_PW_P2    0x13
#define _NRF24_RX_PW_P3    0x14
#define _NRF24_RX_PW_P4    0x15
#define _NRF24_RX_PW_P5    0x16
#define _NRF24_FIFO_STATUS  0x17
#define _NRF24_DYNPD       0x1C
#define _NRF24_FEATURE     0x1D

#define BT1 D4 // GPIO 2
#define BT2 D3 // GPIO 0

byte count;
int channels = 0;
bool jamming = false; // Initialize jamming to false

byte getRegister(byte r) {
    byte c;
    digitalWrite(CSN_PIN, LOW);
    c = SPI.transfer(r & 0x1F);
    c = SPI.transfer(0);
    digitalWrite(CSN_PIN, HIGH);
    return c;
}

void setRegister(byte r, byte v) {
    digitalWrite(CSN_PIN, LOW);
    SPI.transfer((r & 0x1F) | 0x20);
    SPI.transfer(v);
    digitalWrite(CSN_PIN, HIGH);
}

void setup() {
    Serial.begin(57600);
    radio.begin();
    radio.startListening();
    radio.stopListening();

    pinMode(BT1, INPUT_PULLUP);
    pinMode(BT2, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(BT1), pressBt01, FALLING);
    attachInterrupt(digitalPinToInterrupt(BT2), pressBt02, FALLING);

    for (count = 0; count < CHANNELS; count++) {
        channel[count] = 0;
    }

    Serial.println("Starting 2.4GHz Scanner ...");
    Serial.println();
    Serial.println("Channel Layout");

    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.setBitOrder(MSBFIRST);

    pinMode(CE_PIN, OUTPUT);
    disable();
    powerUp();
    setRegister(_NRF24_EN_AA, 0x0);
    setRegister(_NRF24_RF_SETUP, 0x0F); // Set RF setup
}

void powerUp(void) {
    setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) | 0x02);
    delayMicroseconds(130);
}

void disable(void) {
    digitalWrite(CE_PIN, LOW);
}

void enable(void) {
    digitalWrite(CE_PIN, HIGH);
}

void scanChannels(void) {
    disable();
    for (int j = 0; j < 255; j++) {
        for (int i = 0; i < channels; i++) {
            setRegister(_NRF24_RF_CH, (128 * i) / channels);
            enable();
            delayMicroseconds(40);
            disable();
            if (getRegister(_NRF24_RPD) > 0) channel[i]++;
        }
    }
}

void outputChannels(void) {
    int norm = 0;
    for (int i = 0; i < CHANNELS; i++)
        if (channel[i] > norm) norm = channel[i];

    Serial.print('|');
    for (int i = 0; i < CHANNELS; i++) {
        int pos;
        if (norm != 0) pos = (channel[i] * 10) / norm;
        else pos = 0;

        if (pos == 0 && channel[i] > 0) pos++;
        if (pos > 9) pos = 9;

        Serial.print(grey[pos]);
        channel[i] = 0;
    }
    Serial.print("| ");
    Serial.println(norm);
}

void jammer() {
    const char text[] = "xxxxxxxxxxxxxxxx"; // Noise data to send
    for (int i = 0; i < 128; i++) {
        radio.setChannel(i); // Jam all channels from 0 to 127
        radio.write(&text, sizeof(text));
    }
}

void pressBt01() {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > 200) {
        if (channels < 13) {
            channels++;
        } else {
            channels = 0;
        }
    }
    last_interrupt_time = interrupt_time;
}

void pressBt02() {
    jamming = !jamming; // Toggle jamming state
    delay(200); // Debounce delay
}

void loop() {
    Serial.print("Channel: ");
    Serial.println(channels + 1);

    if (jamming) {
        Serial.print("JAMMING ALL CHANNELS ");
        jammer(); // Call the jamming function
    } else {
        scanChannels();
        outputChannels();
    }
}

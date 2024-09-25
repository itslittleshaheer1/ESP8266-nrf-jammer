#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

// Pin configuration for NRF24L01 on ESP8266
#define CE_PIN D1
#define CSN_PIN D2
#define IRQ_PIN D4  // IRQ Pin for NRF24L01

RF24 radio(CE_PIN, CSN_PIN);  // CE -> D1, CSN -> D2

// Addresses for communication
const byte address[6] = "00001";

// Timing variables for sending messages
unsigned long lastSend = 0;
unsigned int sendDelay = 10; // Delay between jamming signals (10 ms)
unsigned long lastJam = 0;
unsigned int jamDelay = 10; // Jamming every 10 ms

volatile bool dataReceived = false;

// Interrupt handler for incoming data on IRQ pin
void ICACHE_RAM_ATTR handleIRQ() {
    if (radio.available()) {
        dataReceived = true;  // Mark that data has been received
    }
}

void setup() {
    // Initialize Serial for debugging
    Serial.begin(9600);
    Serial.println("Initializing NRF24L01...");

    // Initialize the NRF24L01 radio
    if (!radio.begin()) {
        Serial.println("NRF24L01 initialization failed!");
        while (1);
    }

    radio.setPALevel(RF24_PA_MAX);      // Set power level to max for jamming
    radio.setDataRate(RF24_2MBPS);      // Set data rate to 2 Mbps for faster communication
    radio.setRetries(0, 0);             // Disable retries
    radio.setPayloadSize(32);           // Set the payload size to 32 bytes
    radio.setAutoAck(false);            // Disable Auto-Acknowledgment
    radio.openWritingPipe(address);     // Set the address for transmission
    radio.openReadingPipe(1, address);  // Set the address for receiving
    radio.startListening();             // Start in listening mode

    // Attach interrupt to the IRQ pin (D4)
    attachInterrupt(digitalPinToInterrupt(IRQ_PIN), handleIRQ, FALLING);

    Serial.println("NRF24L01 initialized with IRQ.");
}

void loop() {
    unsigned long now = millis();

    // Jamming process
    if (now > lastJam + jamDelay) {
        lastJam = now;

        // Stop listening to transmit noise
        radio.stopListening();

        // Send out noise (for jamming)
        const char *jamSignal = "JAM"; // Simple jamming signal
        bool success = radio.write(jamSignal, sizeof(jamSignal));

        if (success) {
            Serial.println("Jamming signal sent.");
        }

        // Return to listening mode
        radio.startListening();
    }

    // Transmit data every `sendDelay` milliseconds
    if (now > lastSend + sendDelay) {
        lastSend = now;
        const char *message = "hello world";

        // Stop listening and send the message
        radio.stopListening();
        bool success = radio.write(message, sizeof(message));
        
        if (success) {
            Serial.println("Sent: " + String(message));
        } else {
            Serial.println("Failed to send message");
        }

        // Go back to listening mode
        radio.startListening();
    }

    // If data was received via IRQ, process it
    if (dataReceived) {
        dataReceived = false;  // Reset the flag
        char receivedMessage[32] = {0};
        
        // Read the message from the radio
        radio.read(&receivedMessage, sizeof(receivedMessage));
        Serial.println("Received: " + String(receivedMessage));
    }
}

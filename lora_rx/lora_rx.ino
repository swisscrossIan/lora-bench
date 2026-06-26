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
#define RF_BW      62.5
#define RF_SF       12     // must match the tag (lora_radio.ino)
#define RF_CR       8
#define RF_SYNC    0x12
#define RF_PWR      22     // TX-only; kept matching the tag for consistency
#define RF_PRE      8

SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_RST, LORA_BUSY);

// ---- GC1109 front-end module (FEM) — Heltec V4 ONLY ----------------------
// The V4 routes the radio through an external PA/LNA that must be powered and
// TX/RX-switched, or almost no signal reaches the antenna on TX *or* RX. The
// GC1109 needs THREE control lines: FEM_PWR (master power) + FEM_EN held HIGH,
// FEM_TX selecting the path (HIGH = TX PA, LOW = RX). The RX path needs the LNA
// powered too, so FEM_PWR/FEM_EN matter here as well.
// NOTE: V4 LoRa antenna is the FRONT IPEX pad (beside the OLED), not the back.
#define FEM_PWR 7     // GC1109 VFEM_Ctrl — master power, HIGH
#define FEM_EN  2     // GC1109 CSD — enable, HIGH
#define FEM_TX  46    // GC1109 CPS — HIGH = TX PA, LOW = RX
#define RF_TCXO 1.8   // Heltec V4 TCXO reference voltage (DIO3-controlled)

static const uint32_t rfswitch_pins[] = { FEM_TX, RADIOLIB_NC, RADIOLIB_NC, RADIOLIB_NC, RADIOLIB_NC };
static const Module::RfSwitchMode_t rfswitch_table[] = {
  { Module::MODE_IDLE, { LOW  } },
  { Module::MODE_RX,   { LOW  } },
  { Module::MODE_TX,   { HIGH } },
  END_OF_MODE_TABLE,
};

volatile bool gotPacket = false;
void onRx() { gotPacket = true; }

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println(F("\nlora_rx starting..."));
  pinMode(FEM_PWR, OUTPUT); digitalWrite(FEM_PWR, LOW);   // GC1109 master power, ACTIVE-LOW (LOW = powered)
  pinMode(FEM_EN,  OUTPUT); digitalWrite(FEM_EN,  HIGH);   // GC1109 enable
  delay(2);
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  radio.setRfSwitchTable(rfswitch_pins, rfswitch_table);   // drive the TX/RX PA switch

  int st = radio.begin(RF_FREQ, RF_BW, RF_SF, RF_CR, RF_SYNC, RF_PWR, RF_PRE, RF_TCXO);
  if (st != RADIOLIB_ERR_NONE) {
    Serial.print(F("radio.begin failed, code ")); Serial.println(st);
    while (true) delay(1000);
  }
  radio.setDio2AsRfSwitch(true);          // Heltec uses DIO2 for the RF switch
  radio.setRxBoostedGainMode(true);       // ~few dB more RX sensitivity (SX126x)
  radio.setPacketReceivedAction(onRx);
  radio.startReceive();
  Serial.println(F("listening on 915.0 MHz (V4 FEM, boosted RX gain)..."));
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

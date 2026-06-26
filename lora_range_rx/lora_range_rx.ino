// lora_range_rx.ino — Pointage range-test receiver (Heltec WiFi LoRa 32 V4)
// Parses the sequenced packets from lora_range_tx.ino and prints per-packet
// RSSI/SNR plus running packet-error-rate (PER) and average RSSI, so antenna
// and height changes can be compared quantitatively instead of by eye.
//
// V4 hardware notes baked in:
//   - GC1109 front-end powered on GPIO 7/2/46 (RX path needs the LNA powered).
//   - TCXO reference 1.8 V.
//   - RX boosted gain enabled (~few dB extra sensitivity).
// LIBRARY: "RadioLib" by Jan Gromeš.   ANTENNA: front LoRa IPEX (beside OLED).

#include <RadioLib.h>

#define LORA_NSS   8
#define LORA_DIO1  14
#define LORA_RST   12
#define LORA_BUSY  13
#define LORA_SCK    9
#define LORA_MISO  11
#define LORA_MOSI  10

#define RF_FREQ    915.0
#define RF_BW      125.0
#define RF_SF       12
#define RF_CR       7
#define RF_SYNC    0x12
#define RF_PWR      22     // TX-only; kept matching the TX for consistency
#define RF_PRE      8
#define RF_TCXO    1.8

#define FEM_PWR 7          // GC1109 VFEM_Ctrl — master power
#define FEM_EN  2          // GC1109 CSD — enable
#define FEM_TX  46         // GC1109 CPS — TX/RX select

SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_RST, LORA_BUSY);

static const uint32_t rfswitch_pins[] = { FEM_TX, RADIOLIB_NC, RADIOLIB_NC, RADIOLIB_NC, RADIOLIB_NC };
static const Module::RfSwitchMode_t rfswitch_table[] = {
  { Module::MODE_IDLE, { LOW  } },
  { Module::MODE_RX,   { LOW  } },
  { Module::MODE_TX,   { HIGH } },
  END_OF_MODE_TABLE,
};

volatile bool gotPacket = false;
void onRx() { gotPacket = true; }

// ---- running stats ----
uint32_t rxCount = 0;
int32_t  firstSeq = -1;
int32_t  lastSeq  = -1;
float    rssiSum = 0.0f;
float    rssiMin =  999.0f;
float    rssiMax = -999.0f;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println(F("\nlora_range_rx (Heltec V4)..."));

  pinMode(FEM_PWR, OUTPUT); digitalWrite(FEM_PWR, LOW);
  pinMode(FEM_EN,  OUTPUT); digitalWrite(FEM_EN,  HIGH);
  delay(2);

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  radio.setRfSwitchTable(rfswitch_pins, rfswitch_table);

  int st = radio.begin(RF_FREQ, RF_BW, RF_SF, RF_CR, RF_SYNC, RF_PWR, RF_PRE, RF_TCXO);
  if (st != RADIOLIB_ERR_NONE) {
    Serial.print(F("begin failed, code ")); Serial.println(st);
    while (true) delay(1000);
  }

  radio.setDio2AsRfSwitch(true);
  radio.setRxBoostedGainMode(true);        // higher RX sensitivity (SX126x)
  radio.setPacketReceivedAction(onRx);
  radio.startReceive();
  Serial.println(F("listening on 915.0 MHz (boosted RX gain)..."));
}

void loop() {
  if (!gotPacket) return;
  gotPacket = false;

  String data;
  int st = radio.readData(data);
  if (st != RADIOLIB_ERR_NONE) {
    Serial.print(F("readData err ")); Serial.println(st);
    radio.startReceive();
    return;
  }

  float rssi = radio.getRSSI();
  float snr  = radio.getSNR();

  // parse "seq=N"
  int32_t seq = -1;
  int idx = data.indexOf("seq=");
  if (idx >= 0) seq = data.substring(idx + 4).toInt();

  rxCount++;
  if (seq >= 0) {
    if (firstSeq < 0) firstSeq = seq;
    lastSeq = seq;
  }
  rssiSum += rssi;
  if (rssi < rssiMin) rssiMin = rssi;
  if (rssi > rssiMax) rssiMax = rssi;

  int32_t expected = (firstSeq >= 0) ? (lastSeq - firstSeq + 1) : 0;
  int32_t lost     = expected - (int32_t)rxCount;
  float   per      = (expected > 0) ? (100.0f * lost / expected) : 0.0f;

  Serial.print(F("RX seq=")); Serial.print(seq);
  Serial.print(F("  RSSI=")); Serial.print(rssi, 1);
  Serial.print(F("  SNR="));  Serial.print(snr, 1);
  Serial.print(F("  | rx=")); Serial.print(rxCount);
  Serial.print(F("/"));       Serial.print(expected);
  Serial.print(F("  PER="));  Serial.print(per, 1); Serial.print(F("%"));
  Serial.print(F("  avgRSSI=")); Serial.print(rxCount ? rssiSum / rxCount : 0.0f, 1);
  Serial.print(F("  (min ")); Serial.print(rssiMin, 1);
  Serial.print(F(" / max ")); Serial.print(rssiMax, 1); Serial.print(F(")"));
  Serial.println();

  radio.startReceive();
}

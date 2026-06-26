// lora_range_tx.ino — Pointage range-test transmitter (Heltec WiFi LoRa 32 V4)
// Emits a sequenced packet at a fixed cadence so the RX side can compute
// packet-error-rate (PER) and RSSI/SNR vs distance. Pair with lora_range_rx.ino.
//
// V4 hardware notes baked in:
//   - GC1109 front-end needs THREE control lines powered (GPIO 7/2/46).
//   - TCXO reference is 1.8 V on the V4 (DIO3-controlled).
// LIBRARY: "RadioLib" by Jan Gromeš.   ANTENNA: front LoRa IPEX (beside OLED).

#include <RadioLib.h>

// SX1262 pins (Heltec V3/V4)
#define LORA_NSS  8
#define LORA_DIO1 14
#define LORA_RST  12
#define LORA_BUSY 13
#define LORA_SCK  9
#define LORA_MISO 11
#define LORA_MOSI 10

// Radio params — MUST match lora_range_rx exactly (US 915 band)
#define RF_FREQ   915.0
#define RF_BW     125.0
#define RF_SF     12
#define RF_CR     7
#define RF_SYNC   0x12
#define RF_PWR    22       // RadioLib SX1262 ceiling; GC1109 FEM lifts PA output to ~28 dBm
                           // (high-power SKU). Drop to ~10 for close-range bench work.
#define RF_PRE    8
#define RF_TCXO   1.8      // Heltec V4 TCXO reference voltage

// GC1109 front-end (all three control lines)
#define FEM_PWR 7          // VFEM_Ctrl — master power, HIGH
#define FEM_EN  2          // CSD — enable, HIGH
#define FEM_TX  46         // CPS — TX-PA-full (HIGH) / RX-bypass (LOW), RadioLib-driven

#define TX_INTERVAL_MS 1000

SX1262 lora = new Module(LORA_NSS, LORA_DIO1, LORA_RST, LORA_BUSY);

static const uint32_t rfswitch_pins[] = { FEM_TX, RADIOLIB_NC, RADIOLIB_NC, RADIOLIB_NC, RADIOLIB_NC };
static const Module::RfSwitchMode_t rfswitch_table[] = {
  { Module::MODE_IDLE, { LOW  } },
  { Module::MODE_RX,   { LOW  } },
  { Module::MODE_TX,   { HIGH } },
  END_OF_MODE_TABLE,
};

uint32_t seq = 0;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println(F("\nlora_range_tx (Heltec V4)..."));

  pinMode(FEM_PWR, OUTPUT); digitalWrite(FEM_PWR, HIGH);
  pinMode(FEM_EN,  OUTPUT); digitalWrite(FEM_EN,  HIGH);
  delay(2);

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  lora.setRfSwitchTable(rfswitch_pins, rfswitch_table);

  int st = lora.begin(RF_FREQ, RF_BW, RF_SF, RF_CR, RF_SYNC, RF_PWR, RF_PRE, RF_TCXO);
  if (st != RADIOLIB_ERR_NONE) {
    // If this starts failing only after adding RF_TCXO, revert to the 7-arg
    // begin() to isolate a TCXO mismatch.
    Serial.print(F("begin failed, code ")); Serial.println(st);
    while (true) delay(1000);
  }
  lora.setDio2AsRfSwitch(true);
  Serial.print(F("TX ready @ ")); Serial.print(RF_FREQ); Serial.println(F(" MHz"));
}

void loop() {
  uint32_t now = millis();
  String msg = "RANGE seq=" + String(seq) + " t=" + String(now);
  int st = lora.transmit(msg.c_str());
  Serial.print(F("TX seq=")); Serial.print(seq);
  Serial.println(st == RADIOLIB_ERR_NONE ? F(" ok") : F(" FAIL"));
  seq++;
  delay(TX_INTERVAL_MS);
}

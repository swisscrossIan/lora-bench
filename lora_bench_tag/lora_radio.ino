// lora_bench_tag — LoRa transmit tab.
// Sends the tag's payload over the SX1262 (US915) to a paired receiver
// (the lora_rx sketch on the 2nd board). Another tab of the lora_bench_tag
// sketch, so it shares globals/functions via Arduino's concatenation.
//
// REQUIRED LIBRARY (Arduino IDE -> Library Manager):
//   - "RadioLib" by Jan Gromeš
//
// ANTENNA: attach BEFORE powering — transmitting without it can damage the radio.
// Radio params here MUST match lora_rx exactly, or the receiver hears nothing.

#include <RadioLib.h>

// SX1262 pins on Heltec WiFi LoRa 32 V4. These are the documented V3/V4 LoRa
// pins; they sit on their own SPI bus and do not clash with the strip GPIOs
// (42/41/1/38/43/47) or the OLED I2C pins (17/18/21/36). VERIFY if loraInit
// reports an error.
#define LORA_NSS   8
#define LORA_DIO1  14
#define LORA_RST   12
#define LORA_BUSY  13
#define LORA_SCK    9
#define LORA_MISO  11
#define LORA_MOSI  10

// Shared radio params — keep identical to lora_rx (US 915 band, Ontario/NA).
#define RF_FREQ    915.0   // MHz
#define RF_BW      125.0   // kHz
#define RF_SF       12     // spreading factor 7..12 (12 = max range, slower)
#define RF_CR       7      // coding rate 4/7
#define RF_SYNC    0x12    // private-network sync word
#define RF_PWR      22     // dBm (SX1262 max)
#define RF_PRE      8      // preamble length

SX1262 lora = new Module(LORA_NSS, LORA_DIO1, LORA_RST, LORA_BUSY);
bool loraReady = false;

// ---- GC1109 front-end module (FEM) — Heltec V4 ONLY ----------------------
// The V4 (unlike V3) routes the radio through an external PA/LNA that must be
// powered and TX/RX-switched, or almost no power reaches the antenna (this was
// the ~40 dB of missing signal). FEM_EN powers the module (held HIGH while in
// use); FEM_TX selects the TX power amp (HIGH on transmit, LOW on receive).
#define FEM_EN  2     // GC1109 CSD — enable
#define FEM_TX  46    // GC1109 CPS — HIGH = TX PA, LOW = RX

static const uint32_t rfswitch_pins[] = { FEM_TX, RADIOLIB_NC };
static const Module::RfSwitchMode_t rfswitch_table[] = {
  { Module::MODE_IDLE, { LOW  } },
  { Module::MODE_RX,   { LOW  } },
  { Module::MODE_TX,   { HIGH } },
  END_OF_MODE_TABLE,
};

void loraInit() {
  pinMode(FEM_EN, OUTPUT);
  digitalWrite(FEM_EN, HIGH);            // power the GC1109 front-end (V4)
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  lora.setRfSwitchTable(rfswitch_pins, rfswitch_table);   // drive the TX/RX PA switch
  int st = lora.begin(RF_FREQ, RF_BW, RF_SF, RF_CR, RF_SYNC, RF_PWR, RF_PRE);
  if (st != RADIOLIB_ERR_NONE) {
    Serial.print(F("LoRa begin failed, code ")); Serial.println(st);
    loraReady = false;
    return;
  }
  lora.setDio2AsRfSwitch(true);          // Heltec uses DIO2 for the TX/RX RF switch
  loraReady = true;
  Serial.println(F("LoRa ready on 915.0 MHz (V4 FEM enabled)"));
}

// Transmit one record. Called from loop() alongside the Serial.println.
// NOTE: this currently sends the JSON record to prove the link; it will be
// replaced by a compact binary frame before deployment (see docs). transmit()
// blocks a few hundred ms, and a record over 255 bytes fails with
// RADIOLIB_ERR_PACKET_TOO_LONG — a reason NFC free-text stays off LoRa.
void loraSend(const String& msg) {
  if (!loraReady) return;
  int st = lora.transmit(msg.c_str());
  if (st != RADIOLIB_ERR_NONE) {
    Serial.print(F("LoRa TX failed, code ")); Serial.println(st);
  }
}

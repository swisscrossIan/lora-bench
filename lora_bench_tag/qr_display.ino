// lora_bench_tag — OLED QR tab.
// Renders a QR code of the tag's MAC on the onboard SSD1306 while the tag is
// active, and blanks the screen when inactive. This file is a second tab of the
// lora_bench_tag sketch: Arduino concatenates every .ino in the folder into one
// program, so it shares globals (tag_id) and functions with lora_bench_tag.ino.
//
// REQUIRED LIBRARIES (Arduino IDE -> Library Manager):
//   - "U8g2" by oliver
//   - "QRCode" by Richard Moore (ricmoo)
//
// VERIFY THESE PINS for your Heltec WiFi LoRa 32 V4. The values below are the
// documented onboard-OLED pins for the Heltec V3/V4; if your panel stays blank,
// confirm SDA/SCL/RST/Vext against the board's pinout and adjust.
#define OLED_SDA   17
#define OLED_SCL   18
#define OLED_RST   21
#define OLED_VEXT  36   // powers the OLED rail; active LOW on Heltec

#include <Wire.h>
#include <U8g2lib.h>
#include <qrcode.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, OLED_RST);

// The 12-hex MAC fits QR version 1 (21x21 modules). At 2 px/module plus a
// 4-module quiet zone that's 58x58 px — fits the 64px panel and stays scannable.
// (A URL payload would need version 2-3, whose modules get too small to read on
// a 0.96" screen, so we encode the raw MAC and resolve it in the database.)
#define QR_VERSION  1
#define QR_SCALE    2
#define QR_QUIET    4   // modules of quiet zone around the code


void qrInit() {
 pinMode(OLED_VEXT, OUTPUT);
 digitalWrite(OLED_VEXT, LOW);   // enable OLED power rail
 delay(50);
 Wire.begin(OLED_SDA, OLED_SCL);
 oled.begin();
 oled.clearBuffer();
 oled.sendBuffer();              // start blank (tag inactive)
}


void qrClear() {
 oled.clearBuffer();
 oled.sendBuffer();
}


// Draw `text` as a QR, dark modules on a light field (standard polarity).
void qrShow(const char* text) {
 QRCode qrcode;
 uint8_t data[qrcode_getBufferSize(QR_VERSION)];
 qrcode_initText(&qrcode, data, QR_VERSION, ECC_LOW, text);

 const uint8_t mods = qrcode.size + 2 * QR_QUIET;   // total modules incl. border
 const uint8_t side = mods * QR_SCALE;              // pixels
 const int     x0   = (128 - side) / 2;             // center horizontally
 const int     y0   = (64  - side) / 2;             // center vertically

 oled.clearBuffer();
 oled.setDrawColor(1);
 oled.drawBox(x0, y0, side, side);                  // light field + quiet zone
 oled.setDrawColor(0);
 for (uint8_t y = 0; y < qrcode.size; y++) {
   for (uint8_t x = 0; x < qrcode.size; x++) {
     if (qrcode_getModule(&qrcode, x, y)) {
       int px = x0 + (x + QR_QUIET) * QR_SCALE;
       int py = y0 + (y + QR_QUIET) * QR_SCALE;
       oled.drawBox(px, py, QR_SCALE, QR_SCALE);     // erase -> dark module
     }
   }
 }
 oled.setDrawColor(1);
 oled.sendBuffer();
}


void qrShowMac() { qrShow(tag_id); }   // tag_id is defined in lora_bench_tag.ino

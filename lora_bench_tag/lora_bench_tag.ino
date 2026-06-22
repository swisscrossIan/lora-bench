// lora-bench — tear-strip 4-strip acuity (verified-good pins)
// Board: Heltec WiFi LoRa 32 V4. Serial = native USB-CDC @115200.
// Strip1 b56=GPIO42 GREEN | Strip2 b55=GPIO41 YELLOW | Strip3 b53=GPIO39 RED | Strip4 b52=GPIO38 BLACK
// GPIO1 (b51) used due to pin malfunctions, but should be left open for battery monitoring in future. GPIO5(b47) shorted; b48 intermittent.
// Seated=grounded=intact(LOW). Pulled=torn(HIGH). Grounds -> b62/i62 (confirmed GND).


const uint8_t  STRIP_PINS[] = { 42, 41, 1, 38 };   // strip 1..4 = b56,b55,b51,b52
const uint8_t  NUM_STRIPS   = sizeof(STRIP_PINS) / sizeof(STRIP_PINS[0]);
const uint32_t DEBOUNCE_MS  = 50;


// ---- Tag identity ----------------------------------------------------------
// The tag id is the ESP32's full 48-bit factory MAC (12 hex digits). It is
// globally unique with no per-device config; a human-friendly name is mapped
// to this id in the database.
char tagLabel[16];   // full MAC, 12 hex digits + null


int      stableState[NUM_STRIPS];
int      lastRead[NUM_STRIPS];
uint32_t lastEdgeMs[NUM_STRIPS];
int      lastReportedCount = -1;


void initTagId() {
 snprintf(tagLabel, sizeof(tagLabel), "%012llX",
          (unsigned long long)ESP.getEfuseMac());  // full 48-bit factory MAC
}


const char* levelName(int t) {
 switch (t) {
   case 0: return "STANDBY (not activated)";
   case 1: return "ACTIVATED - GREEN";
   case 2: return "ACTIVATED - YELLOW";
   case 3: return "ACTIVATED - RED";
   case 4: return "ACTIVATED - BLACK";
   default: return "(beyond level 4 - reserved)";
 }
}


void setup() {
 Serial.begin(115200);
 delay(300);
 for (uint8_t i = 0; i < NUM_STRIPS; i++) {
   pinMode(STRIP_PINS[i], INPUT_PULLUP);
   stableState[i] = -1;
   lastRead[i]    = -1;
   lastEdgeMs[i]  = 0;
 }
 initTagId();
 Serial.println(F("tear-strip 4-strip acuity test"));
 Serial.print(F("Tag: "));
 Serial.println(tagLabel);
 Serial.println(F("torn count -> acuity colour"));
}


void loop() {
 for (uint8_t i = 0; i < NUM_STRIPS; i++) {
   int reading = digitalRead(STRIP_PINS[i]);
   if (reading != lastRead[i]) {
     lastEdgeMs[i] = millis();
     lastRead[i]   = reading;
   }
   if (millis() - lastEdgeMs[i] > DEBOUNCE_MS) {
     stableState[i] = reading;
   }
 }


 int torn = 0;
 for (uint8_t i = 0; i < NUM_STRIPS; i++) {
   if (stableState[i] == HIGH) torn++;
 }


 if (torn != lastReportedCount) {
   lastReportedCount = torn;
   Serial.print(F("Tag: "));
   Serial.print(tagLabel);
   Serial.print(F("  torn="));
   Serial.print(torn);
   Serial.print(F("  level="));
   Serial.print(torn);
   Serial.print(F("  "));
   Serial.println(levelName(torn));
 }
}

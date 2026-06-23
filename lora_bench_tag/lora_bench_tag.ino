// lora-bench — tear-strip 4-strip acuity (verified-good pins)
// Board: Heltec WiFi LoRa 32 V4. Serial = native USB-CDC @115200.
// Strip1 b56=GPIO42 GREEN | Strip2 b55=GPIO41 YELLOW | Strip3 b53=GPIO39 RED | Strip4 b52=GPIO38 BLACK
// GPIO1 (b51) used due to pin malfunctions, but should be left open for battery monitoring in future. GPIO5(b47) shorted; b48 intermittent.
// Control: i49=GPIO48 deactivate (pull -> tag_status inactive) | i50=GPIO47 transport (pull -> disposition transported)
// Seated=grounded=intact(LOW). Pulled=torn/active(HIGH). Grounds -> b62/i62 (confirmed GND).


const uint8_t  DEACT_PIN      = 43;   // i57 — pull -> tag_status inactive
const uint8_t  TRANSPORT_PIN  = 47;   // i50 — pull -> disposition transported

// First NUM_STRIPS entries are the triage strips; the two control pins follow.
const uint8_t  ALL_PINS[]   = { 42, 41, 1, 38, DEACT_PIN, TRANSPORT_PIN };  // strips=b56,b55,b51,b52
const uint8_t  NUM_PINS     = sizeof(ALL_PINS) / sizeof(ALL_PINS[0]);
const uint8_t  NUM_STRIPS   = 4;
const uint8_t  DEACT_IDX     = 4;
const uint8_t  TRANSPORT_IDX = 5;
const uint32_t DEBOUNCE_MS   = 50;


// ---- Tag identity ----------------------------------------------------------
// The tag id is the ESP32's full 48-bit factory MAC (12 hex digits). It is
// globally unique with no per-device config; a human-friendly name is mapped
// to this id in the database.
char tag_id[16];   // full MAC, 12 hex digits + null


int      stableState[NUM_PINS];
int      lastRead[NUM_PINS];
uint32_t lastEdgeMs[NUM_PINS];

// last emitted state — a record is printed only when something changes
int  lastTorn        = -1;
bool lastActive      = false;
bool lastTransported = false;
bool firstReport     = true;


void initTagId() {
 snprintf(tag_id, sizeof(tag_id), "%012llX",
          (unsigned long long)ESP.getEfuseMac());  // full 48-bit factory MAC
}


const char* triageStatus(int level) {
 switch (level) {
   case 0:  return "none";
   case 1:  return "green";
   case 2:  return "yellow";
   case 3:  return "red";
   case 4:  return "black";
   default: return "reserved";
 }
}


// The transmitted record is now a JSON payload built in payload.ino
// (buildPayload). triageStatus() above feeds its triage_status field.


void setup() {
 Serial.begin(115200);
 delay(300);
 for (uint8_t i = 0; i < NUM_PINS; i++) {
   pinMode(ALL_PINS[i], INPUT_PULLUP);
   stableState[i] = -1;
   lastRead[i]    = -1;
   lastEdgeMs[i]  = 0;
 }
 initTagId();
}


void loop() {
 for (uint8_t i = 0; i < NUM_PINS; i++) {
   int reading = digitalRead(ALL_PINS[i]);
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


 bool deactPulled = (stableState[DEACT_IDX]     == HIGH);
 bool transported = (stableState[TRANSPORT_IDX] == HIGH);
 bool active      = (torn >= 1) && !deactPulled;   // first strip pull -> active; i49 forces inactive


 if (firstReport || torn != lastTorn || active != lastActive || transported != lastTransported) {
   firstReport     = false;
   lastTorn        = torn;
   lastActive      = active;
   lastTransported = transported;
   Serial.println(buildPayload(torn, active, transported));   // JSON record (payload.ino)
 }
}

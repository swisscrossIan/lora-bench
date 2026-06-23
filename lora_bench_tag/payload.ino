// lora_bench_tag — payload builder tab.
// Builds the JSON record that gets transmitted to the backend. This is a second
// tab of the lora_bench_tag sketch (Arduino concatenates every .ino in the
// folder into one program), so it shares globals (tag_id) and functions
// (triageStatus) with lora_bench_tag.ino.
//
// REQUIRED LIBRARY (Arduino IDE -> Library Manager):
//   - "ArduinoJson" by Benoit Blanchon (v6.x)
//
// NULL-WHEN-ABSENT CONTRACT
// Any optional source that is missing, or that can't return data, is emitted as
// JSON null right here on the device — the backend never has to invent a null.
// Every sensor flag below defaults to "absent", so battery/gps/nfc transmit
// null until the real hardware is wired up and sets these globals.

#include <ArduinoJson.h>

uint32_t seq = 0;

// ---- Battery (GPIO1/b51 reserved; not yet read) ----
bool  batteryValid = false;   // set true once an ADC reading is available
int   batteryPct   = 0;

// ---- GPS (optional UART module; not yet wired) ----
bool   gpsPresent  = false;   // hardware detected?
bool   gpsFix      = false;   // valid fix?
double gpsLat      = 0;
double gpsLon      = 0;
float  gpsAcc      = 0;

// ---- NFC (optional reader; not yet wired) ----
bool        nfcPresent = false;   // reader detected?
bool        nfcRead    = false;   // a tag was read this cycle?
const char* nfcUid     = nullptr;
const char* nfcPayload = nullptr;


String buildPayload(int torn, bool active, bool transported) {
  StaticJsonDocument<512> doc;

  doc["tag_id"]        = tag_id;                            // device MAC (12 hex)
  doc["seq"]           = seq++;
  doc["type"]          = "medical";
  doc["torn"]          = torn;                             // tear-strip count
  doc["level"]         = torn;                             // acuity = count
  doc["tag_status"]    = active ? "active" : "inactive";
  doc["triage_status"] = triageStatus(torn);
  doc["disposition"]   = transported ? "transported" : "on_scene";  // always known, never null

  // Battery: null until a reading exists.
  if (batteryValid) doc["battery_pct"] = batteryPct;
  else              doc["battery_pct"] = (const char*)nullptr;

  // GPS three-state: absent -> null | present, no fix -> {fix:false} | fix -> coords.
  if (!gpsPresent) {
    doc["gps"] = (const char*)nullptr;
  } else if (!gpsFix) {
    doc["gps"]["fix"] = false;
  } else {
    JsonObject gps = doc["gps"].to<JsonObject>();
    gps["lat"]        = gpsLat;
    gps["lon"]        = gpsLon;
    gps["accuracy_m"] = gpsAcc;
    gps["fix"]        = true;
  }

  // NFC three-state: absent -> null | present, no tag -> {read:false} | read -> uid/payload.
  if (!nfcPresent) {
    doc["nfc"] = (const char*)nullptr;
  } else if (!nfcRead) {
    doc["nfc"]["read"] = false;
  } else {
    JsonObject nfc = doc["nfc"].to<JsonObject>();
    nfc["uid"]     = nfcUid;       // null -> JSON null if unset
    nfc["payload"] = nfcPayload;
    nfc["read"]    = true;
  }

  String out;
  serializeJson(doc, out);
  return out;
}

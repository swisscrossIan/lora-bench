// lora-bench — PIN SCANNER (both sides) — NEW BOARD, all usable GPIOs from Ian's map
// Touch a row's spare hole to GND -> that row flips to :GND.
// Excluded (onboard hardware/strapping on every V4): GPIO0,3,19,20,21,35,36,37,45 + power/RST rows.
// GPIO5 INCLUDED — it was only shorted on the previous board (solder fault), not a chip limit.

struct Pin { uint8_t gpio; const char* loc; };

const Pin PINS[] = {
  // b-side (left header)
  { 7,  "b45" }, { 6,  "b46" }, { 5,  "b47" }, { 4,  "b48" }, { 2,  "b50" }, { 1,  "b51" },
  { 38, "b52" }, { 39, "b53" }, { 40, "b54" }, { 41, "b55" }, { 42, "b56" }, { 16, "b58" },
  // i-side (right header)
  { 26, "i48" }, { 48, "i49" }, { 47, "i50" }, { 33, "i51" }, { 34, "i52" },
  { 43, "i57" }, { 44, "i58" }
};
const uint8_t N = sizeof(PINS) / sizeof(PINS[0]);
int last[N];

void setup() {
  Serial.begin(115200);
  delay(300);
  for (uint8_t i = 0; i < N; i++) { pinMode(PINS[i].gpio, INPUT_PULLUP); last[i] = -1; }
  Serial.println(F("PIN SCANNER (new board) — touch a row to GND, watch which flips :GND"));
  Serial.println(F("Baseline should be all :---  Anything :GND untouched = shorted, skip it."));
}

void loop() {
  bool changed = false;
  int now[N];
  for (uint8_t i = 0; i < N; i++) {
    now[i] = digitalRead(PINS[i].gpio);
    if (now[i] != last[i]) changed = true;
  }
  if (changed) {
    for (uint8_t i = 0; i < N; i++) {
      last[i] = now[i];
      Serial.print(PINS[i].loc);
      Serial.print(now[i] == LOW ? F(":GND  ") : F(":---  "));
    }
    Serial.println();
  }
  delay(15);
}

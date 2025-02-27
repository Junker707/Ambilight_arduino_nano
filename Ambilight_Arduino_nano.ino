
/*
   Controlling a WS2812 LED strip from a computer + dynamic brightness
*/
//----------------------SETTINGS-----------------------
#define NUM_LEDS 98          // number of LEDs in the strip
#define DI_PIN 13            // pin to which the strip is connected
#define OFF_TIME 10          // time (seconds) after which the strip will turn off if the signal is lost
#define CURRENT_LIMIT 2000   // current limit in milliamps, automatically controls brightness (save your power supply!) 0 - disable limit

#define START_FLASHES 0      // color check on startup (1 - enable, 0 - disable)

#define AUTO_BRIGHT 1        // automatic brightness adjustment based on ambient light level (1 - enable, 0 - disable)
#define MAX_BRIGHT 255       // maximum brightness (0 - 255)
#define MIN_BRIGHT 50        // minimum brightness (0 - 255)
#define BRIGHT_CONSTANT 500  // light intensity adjustment constant (0 - 1023)
// The LOWER the constant, the "sharper" the brightness increase
#define COEF 0.9             // filter coefficient (0.0 - 1.0), the higher the value, the slower the brightness changes
//----------------------SETTINGS-----------------------

int new_bright, new_bright_f;
unsigned long bright_timer, off_timer;

#define serialRate 115200  // communication speed with PC
uint8_t prefix[] = {'A', 'd', 'a'}, hi, lo, chk, i;  // keyword "Ada" for communication
#include <FastLED.h>
CRGB leds[NUM_LEDS];  // create the LED strip
boolean led_state = true;  // flag for LED strip state

void setup()
{
  FastLED.addLeds<WS2812, DI_PIN, GRB>(leds, NUM_LEDS);  // initialize LEDs
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);

  // flashes red, blue, and green on startup (can be disabled)
  if (START_FLASHES) {
    LEDS.showColor(CRGB(255, 0, 0));
    delay(500);
    LEDS.showColor(CRGB(0, 255, 0));
    delay(500);
    LEDS.showColor(CRGB(0, 0, 255));
    delay(500);
    LEDS.showColor(CRGB(0, 0, 0));
  }

  Serial.begin(serialRate);
  Serial.print("Ada\n");     // Connect to PC
}

void check_connection() {
  if (led_state) {
    if (millis() - off_timer > (OFF_TIME * 1000)) {
      led_state = false;
      FastLED.clear();
      FastLED.show();
    }
  }
}

void loop() {
  if (AUTO_BRIGHT) {                         // if adaptive brightness is enabled
    if (millis() - bright_timer > 100) {     // every 100 ms
      bright_timer = millis();               // reset timer
      new_bright = map(analogRead(6), 0, BRIGHT_CONSTANT, MIN_BRIGHT, MAX_BRIGHT);   // read photoresistor data, map range
      new_bright = constrain(new_bright, MIN_BRIGHT, MAX_BRIGHT);
      new_bright_f = new_bright_f * COEF + new_bright * (1 - COEF);
      LEDS.setBrightness(new_bright_f);      // set new brightness
    }
  }
  if (!led_state) led_state = true;
  off_timer = millis();  

  for (i = 0; i < sizeof prefix; ++i) {
waitLoop: while (!Serial.available()) check_connection();;
    if (prefix[i] == Serial.read()) continue;
    i = 0;
    goto waitLoop;
  }

  while (!Serial.available()) check_connection();;
  hi = Serial.read();
  while (!Serial.available()) check_connection();;
  lo = Serial.read();
  while (!Serial.available()) check_connection();;
  chk = Serial.read();
  if (chk != (hi ^ lo ^ 0x55))
  {
    i = 0;
    goto waitLoop;
  }

  memset(leds, 0, NUM_LEDS * sizeof(struct CRGB));
  for (int i = 0; i < NUM_LEDS; i++) {
    byte r, g, b;
    // read data for each color
    while (!Serial.available()) check_connection();
    r = Serial.read();
    while (!Serial.available()) check_connection();
    g = Serial.read();
    while (!Serial.available()) check_connection();
    b = Serial.read();
    leds[i].r = r;
    leds[i].g = g;
    leds[i].b = b;
  }
  FastLED.show();  // write colors to the strip
}
```

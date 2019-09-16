#include "FastLED.h"

#define NUM_STRIPS 6
#define NUM_LEDS_PER_STRIP 11

#define NUM_LEDS 66

#define DATA_PIN 6

#define BUTTON_PIN 2

// default animation frames per second
#define FPS 100

// fire2012 - flame fps; 15 for sauna; 30-100 generally (10-35ms interframe delay)
#define FLAME_FRAMES_PER_SECOND 10

// fire2012 - air cools as it rises; less = taller flames, more = shorter (default 55, suggested range 20-100)
#define COOLING  30

// fire2012 - chance (out of 255) a new spark will be lit; higher = roaring, lower = flickery (default 120, suggested range 50-200)
#define SPARKING 150

// uint8_t max_bright = 128;
#define MAX_BRIGHT 128

CRGB leds[NUM_LEDS];

// rotating "base color" used by many of the patterns
uint8_t gHue = 0;

// global palette, primarily used by fire/flame effect
CRGBPalette16 gPal;

// pre-declare patterns for compiler
extern void fire_simulation_frame();
extern void rainbow();
extern void rainbow_glitter();
extern void solid_gold();
extern void confetti();
extern void confetti_snow();
extern void sinelon();
extern void sinelon_bars();
extern void juggle();

// define patterns - each instance of fire is given a different palette in the loop()
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {
  fire_simulation_frame,
  fire_simulation_frame,
  fire_simulation_frame,
  rainbow,
  rainbow_glitter,
  // solid_gold,
  confetti,
  confetti_snow,
  juggle,
  sinelon,
  sinelon_bars
};
uint8_t gCurrentPatternNumber = 0;

void setup() {

  // sanity delay
  delay(3000);

  // setup serial for debugging
  Serial.begin(9600);
  Serial.println("Hello world!");

  // initialize button pin as input
  pinMode(BUTTON_PIN, INPUT);

  // setup strip
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );

  // restrict brightness re power consumption
  FastLED.setBrightness(MAX_BRIGHT);
  set_max_power_in_volts_and_milliamps(5, 450);

  // palette for fire palette
  gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::OrangeRed, CRGB::Orange);
  // CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), interrupt_handler, HIGH); // CHANGE

}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
void nextPattern() {
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns );
}

void interrupt_handler () {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();

 // assume interrupts faster than 300ms are a bounce and ignore
 if (interrupt_time - last_interrupt_time > 300) {
  nextPattern();
 }
 last_interrupt_time = interrupt_time;
}

void loop() {

  // add entropy to random number generator
  random16_add_entropy(random());

  // display frame of current pattern
  gPatterns[gCurrentPatternNumber]();
  FastLED.show();

  // fire effect with default palette
  if (gCurrentPatternNumber == 0) {
    gPal = CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::OrangeRed, CRGB::Orange);
  }

  // fire effect with ocean palette
  if (gCurrentPatternNumber == 1) {
    gPal = OceanColors_p;
  }

  // fire effect with special palette
  if (gCurrentPatternNumber == 2) {
    gPal = RainbowColors_p;
  }

  // custom delay / fps
  if (gCurrentPatternNumber >= 0 && gCurrentPatternNumber <= 2) {
    FastLED.delay(1000/FLAME_FRAMES_PER_SECOND); // fire_simulation_frame
  } else {
    FastLED.delay(1000/FPS);
  }

  // slowly cycle the 'base color' through the rainbow
  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;
  }

}

/**
 *  Given a 0-based strip_index and a relative_led_index (0..NUM_LEDS_PER_STRIP-1) for that strip, return
 *  the index of the pixel on the whole connected string that corresponds to a bottom-to-top orientation.
 *  Even strips on the stick run bottom-to-top, and odd strips run top-to-bottom so this function computes
 *  the offset and returns the desired index.
 */
int compute_bottom_to_top_offset(int strip_index, int relative_led_index) {
  if ( (strip_index % 2) == 0 ) {
    return (strip_index * NUM_LEDS_PER_STRIP) + relative_led_index;
  } else {
    return (strip_index * NUM_LEDS_PER_STRIP) + (NUM_LEDS_PER_STRIP - 1) - relative_led_index;
  }
}

// based on the legendary fire2012 by Mark Kriegsman, July 2012.
void fire_simulation_frame() {

  static byte heat[NUM_LEDS_PER_STRIP];

  // step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS_PER_STRIP) + 2));
  }

  // step 2. Heat from each cell drifts 'up' and diffuses a little (revised from int k= NUM_LEDS_PER_STRIP - 3)
  for( int k= NUM_LEDS_PER_STRIP; k > 0; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if( random8() < SPARKING ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160,255) );
  }

  // step 4.  Map from heat cells to LED colors
  for( int j = 0; j < NUM_LEDS_PER_STRIP; j++) {

    // scale the heat value from 0-255 down to 0-240 for best results with color palettes
    byte colorindex = scale8(heat[j], 240);
    CRGB pixelcolor = ColorFromPalette(gPal, colorindex);

    // replicate the flame on every strip
    for ( int strip = 0; strip < NUM_STRIPS; strip++ ) {
      leds[compute_bottom_to_top_offset(strip, j)] = pixelcolor;
    }

  }

}

// fastled add glitter
void add_glitter( fract8 chanceOfGlitter) {
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

// fill with solid gold (flashlight mode)
void solid_gold() {
  fill_solid(leds, NUM_LEDS, CRGB::Gold );
}

// fastled rainbow fill applying rotating palette (fastled demo reel)
void rainbow() {
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

// fastled rainbow plus glitter (fastled demo reel)
void rainbow_glitter() {
  rainbow();
  add_glitter(80);
}

// random colored speckles that blink in and fade smoothly (fastled demo reel)
void confetti() {
  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS-1);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

// random white speckles that blink and fade smoothly (modified from fastled demo reel)
void confetti_snow() {
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS-1);
  // leds[pos] += CHSV(0, 200, 255); // red
  leds[pos] += CRGB::White;
}

// colored dot sweeping back and forth, with fading trails, along whole string (fastled demo reel)
void sinelon() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS-1);
  leds[pos] += CHSV(gHue, 255, 192);
}

// colored dot sweeping back and forth, with fading trails, per bar (modified from fastled demo reel)
void sinelon_bars() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(39, 0, NUM_LEDS_PER_STRIP-1);

  for ( int strip = 0; strip < NUM_STRIPS; strip++ ) {
    leds[compute_bottom_to_top_offset(strip, pos)] = CHSV(gHue, 255, 192);
  }
}

// eight colored dots, weaving in and out of sync with each other (fastled demo reel)
void juggle() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS-1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


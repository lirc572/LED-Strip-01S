#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 150
#define DATA_PIN 2

#define DELAYMS 10

CRGB leds[NUM_LEDS];

void setup()
{
  pinMode(DATA_PIN, OUTPUT);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}

void loop()
{
  for (int dot = 0; dot < NUM_LEDS; dot++)
  {
    leds[dot] = CRGB::Red;
    FastLED.show();
    // leds[dot] = CRGB::Black;
    delay(DELAYMS);
  }
  for (int dot = 0; dot < NUM_LEDS; dot++)
  {
    leds[dot] = CRGB::Green;
    FastLED.show();
    // leds[dot] = CRGB::Black;
    delay(DELAYMS);
  }
  for (int dot = NUM_LEDS - 1; dot >= 0; dot--)
  {
    leds[dot] = CRGB::Red;
    FastLED.show();
    // leds[dot] = CRGB::Black;
    delay(DELAYMS);
  }
  for (int dot = NUM_LEDS - 1; dot >= 0; dot--)
  {
    leds[dot] = CRGB::Green;
    FastLED.show();
    // leds[dot] = CRGB::Black;
    delay(DELAYMS);
  }
}

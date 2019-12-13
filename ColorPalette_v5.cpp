#include "FastLED.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN 8
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 37
CRGB leds[NUM_LEDS];
CRGBPalette16 global_CupColor;

int lead_dot = NUM_LEDS;
int Opencircuit = 0;

#define BRIGHTNESS 255
#define FRAMES_PER_SECOND 100

void setup()
{

    Serial.begin(9600);
    delay(3000); // 3 second delay for recovery

    // tell FastLED about the LED strip configuration
    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

    // set master brightness control
    FastLED.setBrightness(BRIGHTNESS);
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();

SimplePatternList gPatterns = {BatteryLed, charging, flashing, sinelon, bpm, rainbowWithGlitter, confetti, juggle};
SimplePatternList gPatterns_OpenCircuit = {charging};

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0;                  // rotating "base color" used by many of the patterns

void loop()
{
    if (checkBatteryStatus() == 0) {
        // Call the current pattern function once, updating the 'leds' array
        gPatterns[gCurrentPatternNumber]();
        // send the 'leds' array out to the actual LED strip
        FastLED.show();
        // insert a delay to keep the framerate modest
        FastLED.delay(1000 / FRAMES_PER_SECOND);
        // do some periodic updates
        EVERY_N_MILLISECONDS(20)
        {
            gHue++; // slowly cycle the "base color" through the rainbow
        }
        EVERY_N_SECONDS(6)
        {
            nextPattern(); // change patterns periodically
        }
    } else {
        gPatterns_OpenCircuit[0]();
        // send the 'leds' array out to the actual LED strip
        FastLED.show();
        // insert a delay to keep the framerate modest
        FastLED.delay(1000 / FRAMES_PER_SECOND);
        // do some periodic updates
        EVERY_N_MILLISECONDS(20)
        {
            gHue++; // slowly cycle the "base color" through the rainbow
        }
        EVERY_N_SECONDS(6)
        {
            nextPattern(); // change patterns periodically
        }
    }
}
//checkBatteryStatus();

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
}

void rainbow()
{
    // FastLED's built-in rainbow generator
    fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter(80);
}

void addGlitter(fract8 chanceOfGlitter)
{
    if (random8() < chanceOfGlitter) {
        leds[random16(NUM_LEDS)] += CRGB::White;
    }
}

void confetti()
{
    // random colored speckles that blink in and fade smoothly
    fadeToBlackBy(leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV(gHue + random8(64), 200, 255);
}
void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy(leds, NUM_LEDS, 10); //20
    int pos = beatsin16(13, 0, NUM_LEDS - 1);
    leds[pos] += CHSV(gHue, 255, 192);
    //leds[pos] += CHSV( 160, 255, 255);
}

void flashing()
{
    EVERY_N_MILLISECONDS(30)
    {
        fadeToBlackBy(leds, NUM_LEDS, 45);
        leds[lead_dot].r = 255;
        lead_dot = lead_dot - 1;
        if (lead_dot == 0) {
            lead_dot = NUM_LEDS;
        }
        FastLED.show();
    }
}

void charging()
{
    if (Opencircuit == 0) {
        memset(leds, 0, NUM_LEDS * 3);
        for (int k = 0; k < 256; k++) {
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i].b = k;
            }
            FastLED.show();
            delay(3);
        }
        for (int k = 255; k >= 0; k--) {
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i].b = k;
            }
            FastLED.show();
            delay(3);
        }
    } else {
        memset(leds, 0, NUM_LEDS * 3);
        for (int k = 0; k < 256; k++) {
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = CRGB::Purple;
                FastLED.show();
            }
        }
    }
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    //CRGBPalette16 palette = PartyColors_p;
    CRGBPalette16 palette = CRGBPalette16(
        //紅(0xFF0000)、橙(0xFF6600)、黃(0xFFFF00)、綠(0x33CC00)、藍(0x3300FF)
        0xFF0000, 0xFF6600, 0xFFFF00, 0x33CC00, 0x3300FF,
        0xFF0000, 0xFF6600, 0xFFFF00, 0x33CC00, 0x3300FF,
        0xFF0000, 0xFF6600, 0xFFFF00, 0x33CC00, 0x3300FF,
        0xFF0000);

    uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
    for (int i = 0; i < NUM_LEDS; i++) { //9948
        leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
    }
}

void juggle()
{
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(leds, NUM_LEDS, 20);
    byte dothue = 0;
    for (int i = 0; i < 8; i++) {
        leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}

int checkBatteryStatus()
{
    if (Opencircuit == 0) {
        uint8_t Battery_Status_1 = random(1, 6);
        uint8_t Battery_Status_2 = random(1, 6);
        uint8_t Battery_Status_3 = random(1, 6);
        uint8_t Battery_Status_4 = random(1, 6);
        uint8_t Battery_Status_5 = random(1, 6);
        uint8_t Battery_Status_6 = random(1, 6);

        uint8_t Battery_Status_ave = (Battery_Status_1 + Battery_Status_2 + Battery_Status_3 + Battery_Status_4 + Battery_Status_5) / 6;
        Serial.println(Battery_Status_ave);

        switch (Battery_Status_1) {
        case 1:
            global_CupColor[1] = CRGB::Red;
            break;
        case 2:
            global_CupColor[1] = CRGB::Orange;
            break;
        case 3:
            global_CupColor[1] = CRGB::Yellow;
            break;
        case 4:
            global_CupColor[1] = CRGB::Green;
            break;
        case 5:
            global_CupColor[1] = CRGB::Blue;
            break;
        }

        switch (Battery_Status_2) {
        case 1:
            global_CupColor[2] = CRGB::Red;
            break;
        case 2:
            global_CupColor[2] = CRGB::Orange;
            break;
        case 3:
            global_CupColor[2] = CRGB::Yellow;
            break;
        case 4:
            global_CupColor[2] = CRGB::Green;
            break;
        case 5:
            global_CupColor[2] = CRGB::Blue;
            break;
        }

        switch (Battery_Status_3) {
        case 1:
            global_CupColor[3] = CRGB::Red;
            break;
        case 2:
            global_CupColor[3] = CRGB::Orange;
            break;
        case 3:
            global_CupColor[3] = CRGB::Yellow;
            break;
        case 4:
            global_CupColor[3] = CRGB::Green;
            break;
        case 5:
            global_CupColor[3] = CRGB::Blue;
            break;
        }

        switch (Battery_Status_4) {
        case 1:
            global_CupColor[4] = CRGB::Red;
            break;
        case 2:
            global_CupColor[4] = CRGB::Orange;
            break;
        case 3:
            global_CupColor[4] = CRGB::Yellow;
            break;
        case 4:
            global_CupColor[4] = CRGB::Green;
            break;
        case 5:
            global_CupColor[4] = CRGB::Blue;
            break;
        }

        switch (Battery_Status_5) {
        case 1:
            global_CupColor[5] = CRGB::Red;
            break;
        case 2:
            global_CupColor[5] = CRGB::Orange;
            break;
        case 3:
            global_CupColor[5] = CRGB::Yellow;
            break;
        case 4:
            global_CupColor[5] = CRGB::Green;
            break;
        case 5:
            global_CupColor[5] = CRGB::Blue;
            break;
        }

        switch (Battery_Status_6) {
        case 1:
            global_CupColor[6] = CRGB::Red;
            break;
        case 2:
            global_CupColor[6] = CRGB::Orange;
            break;
        case 3:
            global_CupColor[6] = CRGB::Yellow;
            break;
        case 4:
            global_CupColor[6] = CRGB::Green;
            break;
        case 5:
            global_CupColor[6] = CRGB::Blue;
            break;
        }

        //For the Charging Sinlon LED
        switch (Battery_Status_ave) {
        case 1:
            global_CupColor[7] = CRGB::Red;
            break;
        case 2:
            global_CupColor[7] = CRGB::Orange;
            break;
        case 3:
            global_CupColor[7] = CRGB::Yellow;
            break;
        case 4:
            global_CupColor[7] = CRGB::Green;
            break;
        case 5:
            global_CupColor[7] = CRGB::Blue;
            break;
        }
    } else {
        global_CupColor[1] = CRGB::Purple;
        global_CupColor[2] = CRGB::Purple;
        global_CupColor[3] = CRGB::Purple;
        global_CupColor[4] = CRGB::Purple;
        global_CupColor[5] = CRGB::Purple;
        global_CupColor[6] = CRGB::Purple;
    }
    return Opencircuit;
}

void BatteryLed()
{
    FastLED.clear();

    for (i = 0; i <7; i++) {
        for (j = 30; j >= 6; j = j - 6) {
            leds[j] = global_CupColor[i];
            leds[j + 1] = global_CupColor[i];
            leds[j + 2] = global_CupColor[i];
            leds[j + 3] = global_CupColor[i];
            leds[j + 4] = global_CupColor[i];
            leds[j + 5] = global_CupColor[i];
            FastLED.show();
            delay(600);
        }
    }

    //LedClear
    memset(leds, 0, NUM_LEDS * 3);
    FastLED.show();
    delay(300);
}

#include "Arduino.h"

#include <NeoPixelBus.h>

#define NUMPIXELS 180
#define NUMPIXELS_FOOT 50



NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(NUMPIXELS, RX);
NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod> strip_foot(NUMPIXELS_FOOT, D4);

RgbColor black(0);

byte config_leds[9] = {0, 150, 150, 150, 0, 0, 150, 150, 0};

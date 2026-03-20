#include "leds.h"


void pisca(int tempo, byte g, byte r, byte b) {
  RgbColor cor(g, r, b);

  for (int i = 1; i < 10; i ++){
    strip.SetPixelColor(i, cor);
    strip_foot.SetPixelColor(i, cor);
  }
  strip.Show();
  strip_foot.Show();
  delay(tempo);
  for (int i = 1; i < 10; i++){
    strip.SetPixelColor(i, black);
    strip_foot.SetPixelColor(i, black);
  }
  strip.Show();
  strip_foot.Show();
  delay(tempo);
}

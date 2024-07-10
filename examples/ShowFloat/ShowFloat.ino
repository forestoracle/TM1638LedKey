#include "TM1638LedKey.h"

uint8_t  strobe = 7;
uint8_t  clock = 9;
uint8_t  data = 8;

uint8_t demoDelay = 50;

TM1638LedKey tm(data, clock, strobe);

void showFloat(void) {
  tm.clear();
  double t = 12.34;
  unsigned char pos = 0;
  for (pos == 1; pos <= 5; pos++) {
    tm.showFloat(t * 100, 2, pos);
    tm.setBrightness(pos);
    delay(500);
    tm.clear();
  }

}
void setup(void) {
  // Устанавливаем яркость
  tm.setBrightness(8);
  Serial.begin(9600);
}


void loop(void) {
  showFloat();
}
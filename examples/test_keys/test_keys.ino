#include "TM1638LedKey.h"

uint8_t strobe = 7;
uint8_t clock = 9;
uint8_t data = 8;

#define LED_DELAY 300

uint8_t demoDelay = 50;

TM1638LedKey tm(data, clock, strobe);

void getbuttons(void) {
    uint32_t keys = 0;
    uint8_t counter;
    keys = tm.getButtons();

    //keys = (keys << 16) | (keys >> 16);

    if (keys != 0) {
        Serial.println(keys, BIN);
    }

    for (uint8_t c = 0; c <= 7; c++) {
        if (bitRead(keys, c)) {
            tm.setLED(c + 1, !tm.getLED(c + 1));
            //Serial.println(c);
        }
    }
}  // getButtons

void setup() {
    // Устанавливаем яркость
    tm.setBrightness(3);
    Serial.begin(9600);
    tm.setLED(8, 1);
    delay(LED_DELAY);
    tm.setLED(7, 1);
    delay(LED_DELAY);
    tm.setLED(6, 1);
    delay(LED_DELAY);
    tm.setLED(5, 1);
    delay(LED_DELAY);
    tm.setLED(4, 1);
    delay(LED_DELAY);
    tm.setLED(3, 1);
    delay(LED_DELAY);
    tm.setLED(2, 1);
    delay(LED_DELAY);
    tm.setLED(1, 1);
    delay(LED_DELAY);
    tm.clear();
}
//
//  Выполняется в цикле
//
void loop() {
    tm.clear();
    delay(100);
    getbuttons();
}

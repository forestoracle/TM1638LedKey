#include <TM1638LedKey.h>

uint8_t strobe = 7;
uint8_t clock = 9;
uint8_t data = 8;

#define LED_DELAY 300

uint8_t demoDelay = 50;

TM1638LedKey tm(data, clock, strobe);

void getButtons(void) {
    tm.getButtonsNonBlocking();                  // опрос кнопок — раз в цикл, без delay()
    uint32_t pressed = tm.getPressedEvents();    // маска только что нажатых кнопок
    uint32_t released = tm.getReleasedEvents();  // маска только что отпущенных кнопок

    for (uint8_t c = 0; c < 8; c++) {
        if (bitRead(pressed, c)) {               // по фронту нажатия переключаем светодиод
            tm.setLED(c + 1, !tm.getLED(c + 1));
        }
    }

    if (pressed != 0) {
        Serial.print("pressed:  ");
        Serial.println(pressed, BIN);
    }
    if (released != 0) {
        Serial.print("released: ");
        Serial.println(released, BIN);
    }
}  // getButtons

void setup() {
    uint8_t count;
    // Устанавливаем яркость
    tm.setBrightness(3);
    Serial.begin(9600);
    for (count = 1; count <= 8; count++) {
        tm.setLED(count, 1);
        delay(LED_DELAY);
    }

    tm.clear();
}
//
//  Выполняется в цикле
//
void loop() {
    tm.clear();
    delay(100);
    getButtons();
}

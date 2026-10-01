#include <TM1638LedKey.h>

/*
  Пример работы с кнопками модуля TM1638 Led&Key.

  Показано неблокирующее чтение и события кнопок:
    - getButtonsNonBlocking()             — опрос кнопок без delay() (раз в цикл);
    - buttonPressed() / buttonReleased()  — событие по одной кнопке;
    - getPressedEvents() / getReleasedEvents() — битовые маски событий.

  Действия:
    - нажатие кнопки переключает её светодиод;
    - отпускание кнопки 8 очищает индикатор;
    - маски нажатых/отпущенных кнопок печатаются в Serial (9600).

  Типовое подключение модуля:
    strobe = 7;
    clock  = 9;
    data   = 8;
*/

uint8_t strobe = 7;
uint8_t clock = 9;
uint8_t data = 8;

TM1638LedKey tm(data, clock, strobe);

void setup(void) {
    tm.setBrightness(4);
    Serial.begin(9600);
}

void loop(void) {
    tm.getButtonsNonBlocking();  // опрос кнопок — раз в цикл, без delay()

    // Per-button: по нажатию кнопки переключаем её светодиод
    for (uint8_t c = 1; c <= 8; c++) {
        if (tm.buttonPressed(c)) {
            tm.setLED(c, !tm.getLED(c));
        }
    }

    // Per-button: отпускание кнопки 8 очищает индикатор
    if (tm.buttonReleased(8)) {
        tm.clear();
    }

    // Маски событий за текущий цикл (бит 0 -> кнопка 1, ..., бит 7 -> кнопка 8)
    uint32_t pressed = tm.getPressedEvents();
    uint32_t released = tm.getReleasedEvents();
    if (pressed != 0) {
        Serial.print("pressed:  ");
        Serial.println(pressed, BIN);
    }
    if (released != 0) {
        Serial.print("released: ");
        Serial.println(released, BIN);
    }
}

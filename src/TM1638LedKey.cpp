/*
  Библиотека управления модулем TM1638 Led&Key

  Версия: 0.9
  Дата:   2026-10-01

*/
#include "TM1638LedKey.h"
/*
*  Конструктор с пользовательским подключением (можно установить пины, к которым подключен модуль)
*  Выключен режим автоинкремента адреса (см. функцию reset)
*/
TM1638LedKey::TM1638LedKey(uint8_t data, uint8_t clock, uint8_t strobe) {
    dataPin = data;
    clockPin = clock;
    strobePin = strobe;
    // Настраиваем выводы МК
    init();
    reset();  //
}

/*
*  Конструктор с типовым подключением
*  Типовое поключение модуля
*    strobePin = 7;
*    clockPin  = 9;
*    dataPin   = 8;
*  Выключен режим автоинкремента адреса (см. функцию reset)
*/
TM1638LedKey::TM1638LedKey(void) {
    dataPin = 8;    // типовое подключение модуля
    clockPin = 9;   //  -
    strobePin = 7;  //  -
    // Настраиваем выводы МК
    init();   // настройка выводов МК
    reset();  // настройка модуля TM1638
}

/*
*  Инициализация пинов
*  В начале работы все пины будут настроены на вывод.
*/
void TM1638LedKey::init(void) {
    pinMode(strobePin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);
}

/*
*  Очистка регистров
*  Обратить внимание, что включен режим автоинкремента адреса.
*  Каждый следующий байт посылается в следующий новый адрес
*/
void TM1638LedKey::reset(void) {
    setAutoincOn();                                           // на время инициализации включаем автоинкремент адреса
    digitalWrite(strobePin, LOW);                             // опускаем строб
    shiftOut(dataPin, clockPin, LSBFIRST, ADR_TM1638_START);  // начальный адрес = 0xC0
    for (uint8_t i = 0; i < 16; i++) {
        shiftOut(dataPin, clockPin, LSBFIRST, 0x00);
    }
    digitalWrite(strobePin, HIGH);
    leds = B00000000;  // все светодиоды выключены
    setAutoincOff();   // автоинкремент адреса отключен
}

/*
*  Посылка команды
*    1. Перед посылкой команды нужно подать на строб низкий уровень сигнала.
*    2. Послать команду
*    3. Установить на стробе высокий уровень сигнала
*/
void TM1638LedKey::sendCommand(uint8_t command) {
    digitalWrite(strobePin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, command);
    digitalWrite(strobePin, HIGH);
}
/*
*  Посылка символа
*    1. Перед посылкой команды нужно подать на строб низкий уровень сигнала.
*    2. Послать адрес
*    3. Послать символ
*    3. Установить на стробе высокий уровень сигнала
*/
void TM1638LedKey::sendSymbol(uint8_t addr, uint8_t symbol) {
    digitalWrite(strobePin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, addr);    // передаем адрес СВ
    shiftOut(dataPin, clockPin, LSBFIRST, symbol);  // передаем состояние СВ
    digitalWrite(strobePin, HIGH);
}

/*
*  Чтение кнопок.
*
*  Модуль отдаёт 4 байта ключевого сканирования. В байте i линия KS(2i+1)
*  находится в бите 0, линия KS(2i+2) — в бите 4 (данные идут младшим битом вперёд,
*  поэтому читаем через shiftIn с порядком LSBFIRST).
*  На плате LED&KEY кнопки подключены к линиям KS «шахматкой»:
*      S1=KS1, S2=KS3, S3=KS5, S4=KS7, S5=KS2, S6=KS4, S7=KS6, S8=KS8.
*  Функция преобразует линии KS в физические номера кнопок.
*  Возвращаемая маска: бит 0 = кнопка 1 (крайняя слева), бит 7 = кнопка 8 (крайняя справа).
*/
uint32_t TM1638LedKey::buttons(void) {
    uint8_t raw[4];      // 4 байта ключевого сканирования
    uint32_t keys = 0;

    digitalWrite(strobePin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, CMD_TM1638_KEY_SCAN);

    digitalWrite(clockPin, LOW);  // нужно для работы функции shiftIn, т.к. она подразумевает передачу данных по спадающему сигналу clock,
    // а у модуля он нарастающий. См. документацию к функции shiftIn
    pinMode(dataPin, INPUT);
    digitalWrite(dataPin, HIGH);

    for (uint8_t i = 0; i < 4; i++) {
        raw[i] = shiftIn(dataPin, clockPin, LSBFIRST);
    }

    pinMode(dataPin, OUTPUT);
    digitalWrite(dataPin, LOW);
    digitalWrite(strobePin, HIGH);

    for (uint8_t i = 0; i < 4; i++) {
        if (raw[i] & 0x01) {  // линия KS(2i+1) -> кнопки 1..4
            keys |= (1UL << i);
        }
        if (raw[i] & 0x10) {  // линия KS(2i+2) -> кнопки 5..8
            keys |= (1UL << (i + 4));
        }
    }

    return keys;
}  // buttons

/*
*  Установка яркости
*/
void TM1638LedKey::setBrightness(uint8_t brightness) {
    uint8_t inc = (brightness > 8) ? 8 : brightness;  // если передано число > 8, то дисплей на полную яркость
    sendCommand(CMD_TM1638_DISPLAY_OFF + inc);
    // Интересно, что если написать так sendCommand(CMD_TM1638_DISPLAY_OFF + (brightness > 8)? 8: brightness);
    // то программа занимает на 8 байт больше места, размер переменных не меняется
}

/*
*  Включение автоинкремента адреса
*/
void TM1638LedKey::setAutoincOn(void) {
    sendCommand(CMD_TM1638_SET_AUTOINC_ON);
}

/*
*  Отключение автоинкремента адреса
*/
void TM1638LedKey::setAutoincOff(void) {
    sendCommand(CMD_TM1638_SET_AUTOINC_OFF);
}

/*
*  Возвращает адрес СД.
*  -------------------------
*    num - номер разряда.
*  Может принимать значения от 1 до 8 (нумерация слева направо как на плате).
*  Если передано значение больше 8, то возвращается адрес последнего 8-го СД.
*/
uint8_t TM1638LedKey::getLEDAddress(uint8_t num) {
    uint8_t n = (num > 8) ? 8 : num;
    return ADR_TM1638_START + 1 + (n - 1) * 2;
}
/*
*  Возвращает адрес разряда.
*  -------------------------
*    num - номер разряда.
*  Может принимать значения от 1 до 8 (нумерация слева направо как на плате).
*  Если передано значение больше 8, то возвращается адрес последнего 8-го разряда.
*/
uint8_t TM1638LedKey::getGridAddress(uint8_t num) {
    uint8_t n = (num < 1) ? 1 : num;  // зажимаем номер в диапазон 1..8
    n = (n > DIGITS) ? DIGITS : n;
    return ADR_TM1638_START + (n - 1) * 2;
}

/*
*  Установка состояния светодиода
*    num - номер светодиода
*    on  - true|false
*  Нумерация слева направо от 1 до 8
*  Регистрируем состояние светодиода в переменной leds
*  для последующего чтения
*/
void TM1638LedKey::setLED(uint8_t num, uint8_t on) {
    uint8_t n = (num < 1) ? 1 : num;  // зажимаем номер в диапазон 1..8
    n = (n > DIGITS) ? DIGITS : n;
    on = (on > 1) ? 1 : on;
    if (on) {  // регистрируем состояние СВ
        leds |= (1 << (n - 1));
    } else {
        leds &= ~(1 << (n - 1));
    }
    sendSymbol(getLEDAddress(n), on);
}
/*
*  Получение состояния СВ
*    true  - включен
*    false - выключен
*  Нумерация слева направо от 1 до 8
*  Нужно учитывать, что возвращается не реальное состояние СВ, а бит частной переменной leds
*/
bool TM1638LedKey::getLED(uint8_t num) {
    uint8_t n = (num < 1) ? 1 : num;  // зажимаем номер в диапазон 1..8
    n = (n > DIGITS) ? DIGITS : n;
    return bool((1 << (n - 1)) & leds);
}

void TM1638LedKey::setGrid(uint8_t grid, uint8_t val, bool dp) {
    uint8_t gridAddr = getGridAddress(grid);
    uint8_t symbol = (val < sizeof(NUMBER_FONT)) ? NUMBER_FONT[val] : NUMBER_FONT[CLEAR];  // защита от выхода за пределы таблицы
    if (dp) {
        symbol |= (1 << 7);
    }
    sendSymbol(gridAddr, symbol);
}  // setGrid


uint32_t TM1638LedKey::getButtons(void) {
    uint32_t keys0 = buttons();
    delay(DEBOUNCE_DELAY);
    uint32_t keys1 = buttons();
    if (keys0 == keys1) return keys0; else return 0;

}

/*
*  Неблокирующее чтение кнопок с антидребезгом.
*  В отличие от getButtons(), не вызывает delay(): функцию нужно регулярно
*  вызывать в loop(). Состояние считается стабильным, если прочитанные значения
*  не менялись не менее DEBOUNCE_DELAY мс.
*  Возвращает маску стабильно нажатых кнопок (бит 0 = кнопка 1).
*/
uint32_t TM1638LedKey::getButtonsNonBlocking(void) {
    uint32_t now = millis();
    uint32_t raw = buttons();  // чтение без задержек (shiftIn, единицы мкс)
    prevKeys = debouncedKeys;  // состояние с прошлого опроса (для детектирования фронта)
    if (raw != lastRawKeys) {          // показания изменились — перезапускаем отсчёт
        lastRawKeys = raw;
        lastChangeMs = now;
    } else if (now - lastChangeMs >= DEBOUNCE_DELAY) {
        debouncedKeys = raw;           // показания стабильны — фиксируем
    }
    return debouncedKeys;
}

/*
*  Маска кнопок, только что нажатых (фронт 0 -> 1) в текущем цикле.
*  Требует вызова getButtonsNonBlocking() в текущем цикле.
*  Бит 0 = кнопка 1, ..., бит 7 = кнопка 8.
*/
uint32_t TM1638LedKey::getPressedEvents(void) {
    return debouncedKeys & ~prevKeys;
}

/*
*  Маска кнопок, только что отпущенных (фронт 1 -> 0) в текущем цикле.
*  Требует вызова getButtonsNonBlocking() в текущем цикле.
*  Бит 0 = кнопка 1, ..., бит 7 = кнопка 8.
*/
uint32_t TM1638LedKey::getReleasedEvents(void) {
    return prevKeys & ~debouncedKeys;
}

/*
*  Момент нажатия кнопки (фронт 0 -> 1).
*  Требует вызова getButtonsNonBlocking() в текущем цикле.
*    num - номер кнопки 1..8 (зажимается в диапазон).
*/
bool TM1638LedKey::buttonPressed(uint8_t num) {
    uint8_t n = (num < 1) ? 1 : num;
    n = (n > DIGITS) ? DIGITS : n;
    return (getPressedEvents() & (1UL << (n - 1))) != 0;
}

/*
*  Момент отпускания кнопки (фронт 1 -> 0).
*  Требует вызова getButtonsNonBlocking() в текущем цикле.
*    num - номер кнопки 1..8 (зажимается в диапазон).
*/
bool TM1638LedKey::buttonReleased(uint8_t num) {
    uint8_t n = (num < 1) ? 1 : num;
    n = (n > DIGITS) ? DIGITS : n;
    return (getReleasedEvents() & (1UL << (n - 1))) != 0;
}

void TM1638LedKey::setGauges(uint8_t gauge1, uint8_t gauge2, uint8_t gauge3, uint8_t gauge4, uint8_t gauge5) {
    uint8_t const seg1 = B00000001;
    uint8_t const seg3 = B01000000;
    uint8_t const seg5 = B00001000;

    uint8_t const seg2Half = B00100000;
    uint8_t const seg2Full = B00100010;

    uint8_t const seg4Half = B00010000;
    uint8_t const seg4Full = B00010100;

    uint8_t segs;

    for (uint8_t counter = 1; counter <= 8; counter++) {
        segs = 0;
        // 8
        if (counter <= gauge1) {
            segs |= seg1;
        }
        if (counter <= gauge3) {
            segs |= seg3;
        }
        if (counter <= gauge5) {
            segs |= seg5;
        }
        // 2-16
        if (counter <= gauge2 >> 1) {
            segs |= seg2Full;
        }
        if ((gauge2 % 2 == 1) && ((counter - 1) == (gauge2 >> 1))) {
            segs |= seg2Half;
        }
        // 4-16
        if (counter <= gauge4 >> 1) {
            segs |= seg4Full;
        }
        if ((gauge4 % 2 == 1) && ((counter - 1) == (gauge4 >> 1))) {
            segs |= seg4Half;
        }
        sendSymbol(getGridAddress(counter), segs);
    }
}  // setGauges

void TM1638LedKey::clear(void) {
    for (int counter = 1; counter <= 8; counter++) {
        setGrid(counter, CLEAR, false);
    }
}

void TM1638LedKey::showFloat(int32_t val, uint8_t dp, uint8_t pos) {
    uint8_t counter, curPos;
    char vals[DIGITS + 1];  // +1 под завершающий '\0'
    bool negative = (val < 0);
    uint32_t magnitude = negative ? (0u - (uint32_t)val) : (uint32_t)val;  // модуль числа (корректно и для INT32_MIN)
    snprintf(vals, sizeof(vals), "%lu", (unsigned long)magnitude);
    counter = 0;
    curPos = (pos < 1) ? 1 : pos;  // зажимаем стартовый разряд в диапазон 1..8
    curPos = (curPos > DIGITS) ? DIGITS : curPos;

    if (negative && curPos <= DIGITS) {  // знак "-" занимает отдельный разряд
        setGrid(curPos, MINUS, false);
        curPos++;
    }

    while (counter < DIGITS && vals[counter] != '\0' && curPos <= DIGITS) {
        setGrid(curPos, vals[counter] - '0', (counter + 1 == dp));
        counter++;
        curPos++;
    }
}

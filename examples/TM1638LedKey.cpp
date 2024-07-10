/*
  Библиотека управления модулем TM1638 Led&Key

  Версия: 0.5
  Дата:   2024-07-09

*/
#include "TM1638LedKey.h"
/*
*  Конструктор с пользовательским подключением (можно установить пины, к которым подключен модуль)
*  Выключен режим автоинкремента адреса (см. функцию reset)
*/
TM1638LedKey::TM1638LedKey(uint8_t data, uint8_t clock, uint8_t strobe)
{
    dataPin   = data;
    clockPin  = clock;
    strobePin = strobe;
    // Настраиваем выводы МК
    init();
    reset();                             //
}

/*
*  Конструктор с типовым подключением
*  Типовое поключение модуля
*    strobePin = 7;
*    clockPin  = 9;
*    dataPin   = 8;
*  Выключен режим автоинкремента адреса (см. функцию reset)
*/
TM1638LedKey::TM1638LedKey(void)
{
    dataPin   = 8;                         // типовое подключение модуля
    clockPin  = 9;                         //  -
    strobePin = 7;                         //  -
    // Настраиваем выводы МК
    init();                                // настройка выводов МК
    reset();                               // настройка модуля TM1638
}

/*
*  Инициализация пинов
*  В начале работы все пины будут настроены на вывод.
*/
void TM1638LedKey::init(void)
{
    pinMode(strobePin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);
}

/*
*  Очистка регистров
*  Обратить внимание, что включен режим автоинкремента адреса.
*  Каждый следующий байт посылается в следующий новый адрес
*/
void TM1638LedKey::reset(void)
{
    setAutoincOn();                                             // на время инициализации включаем автоинкремент адреса
    digitalWrite(strobePin, LOW);                               // опускаем строб
    shiftOut(dataPin, clockPin, LSBFIRST, ADR_TM1638_START);    // начальный адрес = 0xC0
    for (uint8_t i = 0; i < 16; i++) {
        shiftOut(dataPin, clockPin, LSBFIRST, 0x00);
    }
    digitalWrite(strobePin, HIGH);
    leds = B00000000;                                           // все светодиоды выключены
    setAutoincOff();                                            // автоинкремент адреса отключен
}

/*
*  Посылка команды
*    1. Перед посылкой команды нужно подать на строб низкий уровень сигнала.
*    2. Послать команду
*    3. Установить на стробе высокий уровень сигнала
*/
void TM1638LedKey::sendCommand(uint8_t command)
{
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
void TM1638LedKey::sendSymbol(uint8_t addr, uint8_t symbol)
{
    digitalWrite(strobePin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, addr);    // передаем адрес СВ
    shiftOut(dataPin, clockPin, LSBFIRST, symbol);  // передаем состояние СВ
    digitalWrite(strobePin, HIGH);
}

/*
*  Установка яркости
*/
void TM1638LedKey::setBrightness(uint8_t brightness)
{
    uint8_t inc = (brightness > 8)? 8: brightness; // если передано число > 8, то дисплей на полную яркость
    sendCommand(CMD_TM1638_DISPLAY_OFF + inc);
    // Интересно, что если написать так sendCommand(CMD_TM1638_DISPLAY_OFF + (brightness > 8)? 8: brightness);
    // то программа занимает на 8 байт больше места, размер переменных не меняется
}

/*
*  Обновление состояния всех СВ в соответствие с частной переменной leds
*  В этой переменной будем хранить состояние СВ, которое устанавливается в функции setLED.
*  Биты, соотвествующие СВ, возвращаются функцией getLED
*/
void TM1638LedKey::setLEDs(void)
{
  // TODO: 
}

/*
*  Включение автоинкремента адреса
*/
void TM1638LedKey::setAutoincOn(void)
{
    sendCommand(CMD_TM1638_SET_AUTOINC_ON);
}

/*
*  Отключение автоинкремента адреса
*/
void TM1638LedKey::setAutoincOff(void)
{
    sendCommand(CMD_TM1638_SET_AUTOINC_OFF);
}

/*
*  Возвращает адрес СД.
*  -------------------------
*    num - номер разряда.
*  Может принимать значения от 1 до 8 (нумерация слева направо как на плате).
*  Если передано значение больше 8, то возвращается адрес последнего 8-го СД.
*/
uint8_t TM1638LedKey::getLEDAddress(uint8_t num)
{
    uint8_t n = (num > 8)? 8: num;
    return ADR_TM1638_START + 1 + (n-1) * 2;
}
/*
*  Возвращает адрес разряда.
*  -------------------------
*    num - номер разряда.
*  Может принимать значения от 1 до 8 (нумерация слева направо как на плате).
*  Если передано значение больше 8, то возвращается адрес последнего 8-го разряда.
*/
uint8_t TM1638LedKey::getGridAddress(uint8_t num)
{
    uint8_t n = (num > 8)? 8: num;
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
void TM1638LedKey::setLED(uint8_t num, uint8_t on)
{
    uint8_t n = num;
    n = (num > 8) ? 8 : num;
    n = (num == 0) ? 1: num;
    on = (on > 1) ? 1: on;
    if (on) {                                                // регистрируем состояние СВ
        leds |= (1 << n-1);
    } else {
        leds &= ~(1 << n-1);
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
bool TM1638LedKey::getLED(uint8_t num)
{
    uint8_t n = num;
    n = (num > DIGITS) ? DIGITS : num;
    n = (num < 1) ? 1 : num;
    return bool((1 << n-1)  &  leds);
}

void TM1638LedKey::setGrid(uint8_t grid, uint8_t val, bool dp = false)
{
    uint8_t gridAddr = getGridAddress(grid);
    uint8_t symbol   = NUMBER_FONT[val];
    if (dp) {
        symbol |= (1 << 7);
    }
    sendSymbol(gridAddr, symbol);
} // setGrid

/*
*  Чтение кнопок.
*
*
*  Например, возвращаемое значение содержит 4 байта. На модуле 8 кнопок, хватает байта.
*  Но! Модуль может обрабатывать до 24 кнопок.
*/
uint32_t TM1638LedKey::getButtons(void)
{
    // TODO (spronin#1#):  разобраться с этой переменной. Она избыточна, диодов то всего 8
    uint32_t keys = 0;  //
    uint8_t tmp = 0;

    digitalWrite(strobePin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, CMD_TM1638_KEY_SCAN);

    digitalWrite(clockPin, LOW);  // нужно для работы функции shiftIn, т.к. она подразумевает передачу данных по спадающему сигналу clock,
    // а у модуля он нарастающий. См. документацию к функции shiftIn
    pinMode(dataPin, INPUT);
    digitalWrite(dataPin, HIGH);
    for (uint8_t i = 0; i < 4; i++) {
        tmp = shiftIn(dataPin, clockPin, MSBFIRST);
        if (tmp > 0) {
            tmp >>= i;
        }
        keys |= (uint32_t)tmp;
    }
    pinMode(dataPin, OUTPUT);
    digitalWrite(dataPin, LOW);
    digitalWrite(strobePin, HIGH);
    return keys;
} // getButtons


void TM1638LedKey::setGauges(uint8_t gauge1, uint8_t gauge2, uint8_t gauge3, uint8_t gauge4, uint8_t gauge5)
{
    uint8_t seg1     = B00000001;
    uint8_t seg3     = B01000000;
    uint8_t seg5     = B00001000;

    uint8_t seg2Half = B00100000;
    uint8_t seg2Full = B00100010;

    uint8_t seg4Half = B00010000;
    uint8_t seg4Full = B00010100;

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
        if ((gauge2 % 2 == 1) && ((counter-1) == (gauge2 >> 1))) {
            segs |= seg2Half;
        }
        // 4-16
        if (counter <= gauge4 >> 1) {
            segs |= seg4Full;
        }
        if ((gauge4 % 2 == 1) && ((counter-1) == (gauge4 >> 1))) {
            segs |= seg4Half;
        }
        sendSymbol(getGridAddress(counter), segs);
    }
} // setGauges

void TM1638LedKey::clear(void)
{
    for (int counter = 1; counter <= 8; counter++) {
        setGrid(counter, CLEAR, false );
    }
}

void TM1638LedKey::showFloat(uint32_t val, uint8_t dp, uint8_t pos)
{
    uint8_t counter, curPos;
    char vals[DIGITS];
    snprintf(vals, 9, "%ld", val);
    Serial.println(val);
    counter = 0;
    curPos = (pos < 1) ? 1 : pos;
    curPos = (pos > DIGITS) ? DIGITS : pos;

    while (counter < DIGITS && vals[counter] != '\0' && curPos <= DIGITS) {
        //for (counter = 0; counter < 8; counter++) {
        if (vals[counter] == '-') {
            setGrid(curPos, MINUS, (counter + 1 == dp) );
        } else {
            setGrid(curPos, vals[counter] - 48, (counter + 1 == dp) );
        }
        counter++;
        curPos++;
    }
}


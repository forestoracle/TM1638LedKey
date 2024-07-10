/*
  Библиотека управления модулем TM1638 Led&Key

  Версия: 0.5
  Дата:   2024-07-09

  История:
  0.5 2024-07-09  - некоторые улучшения кода
  0.4 2018-02-16  - вывод целого числа в указанной позиции и простановка точки
  0.3 2018-02-15  - добавлена возможность выводить десятичную точку (setGrid)


  На борту модуля:
    8 светодиодов (номера 1-8);
    8 семисегментных индикаторов (номера 1-8);
    8 кнопок (номера 1-8).

  Типовое подключение
  ---------------------
  VCC   - +5V
  GND   - Ground
  STB   - строб (pin 7)
  CLK   - синхронизация (pin 9)
  DIO   - двунаправленный интерфейс данных и команд (pin 8)

  ---------------------------------------------------------------------------------------
  Условимся, что нумерация разрядов (gauge), кнопок и диодов начинается с 1.
  Счет проводится слева направо, как это отображено на плате TM1638.
  Если в качестве аргумента по количеству заженных диодов передано 0, то ничего не горит.
  Гасятся все разряды, все диоды.
  ---------------------------------------------------------------------------------------

*/
/*
  Команда адресации 0xCX
  ----------------------
  Для адресации СД и разрядов (gauge) используется команда 0xCX.
  Старшие биты (с B7 по B4) команды 0xCX устанавливают в старшей половине байта значение C.
  Младшие 4 бита (с B3 по B0) используются для передачи адреса регистра, с помощью которых происходит управление СД и разрядами.
  Далее приводится таблица с адресами регистров управления СД и разрядами.
  ---------------------
  Адрес       Описание
  ---------------------
  0x00 (0000) Разряд 1
  0x01 (0001) LED 1
  0x02 (0010) Разряд 2
  0x03 (0011) LED 2
  0x04 (0100) Разряд 3
  0x05 (0101) LED 3
  0x06 (0110) Разряд 4
  0x07 (0111) LED 4
  0x08 (1000) Разряд 5
  0x09 (2001) LED 5
  0x0a (1010) Разряд 6
  0x0b (1011) LED 6
  0x0c (1100) Разряд 7
  0x0d (1101) LED 7
  0x0e (1110) Разряд 8
  0x0f (1111) LED 8
  ---------------------

  Таким образом, каждый четный адрес является адресом регистра разряда, а каждый нечетный является адресом регистра светодиода.
  На самом деле модуль предназначен для управления индикаторами, имеющими до 10 сегментов, т.е. каждый светодиод - это по сути 9 сегмент (SEG9)
  соотвествующего разряда. Просто четные регистры управляют сегментами от 1 до 8, а младшие 2 бита нечетных регистров - разрядами 9 и 10.

  Т.е. теоретически можно подключить еще одну линейку из 8 светодиодов, выполняющих роль 10-го разряда, на линию управления SEG10.

  Предположим, что мы хотим включить светодиод LED 2. Тогда мы должны отдать команду 0xC3, которая укажет,
  что следующий за данной командой байт должен быть помещен в регистр с адресом 0x03.
  Сразу после записи команды мы должны передать 1, и она будет помещена в регистр 0x03 - включится светодиод LED 2.
  Для того, чтобы выключить LED 2, мы вместо 1 посылаем 0 - светодиод LED 2 выключится.

  Подобным образом происходит и управление разрядами, только вместо 1 и 0 (включить и выключить) мы должны записать в управляющий регистр
  байт, каждый бит которого включает или выключает сегменты разряда.

  Чтение кнопок
  -------------
  Микросхема TM1638 может отслеживать нажатие до 32 кнопок.
  Для чтения состояния кнопок предусмотрена команда 0x42.
  После отправки этой команды нужно прочитать 4 байта.
  Каждый бит установленный в 1 будет обозначать нажатие кнопки.
*/
/*
  Светодиоды
  ---------------------------
  Работа со светодиодами (СД) заключается в следующем:
  При изменении состояния СД изменяется приватная переменная leds
  Из ее битов можно прочитать состояние СД при помощи функции getLED

   Индикаторы уровня
   -------------------------------------------
   Индикаторы уровня числом 6

    0   0   0   0   0   0   0   0    <- gaugeLED
    -   -   -   -   -   -   -   -    <- gauge8_1
   | | | | | | | | | | | | | | | |   <- gauge16_2
    -   -   -   -   -   -   -   -    <- gauge8_3
   | | | | | | | | | | | | | | | |   <- gauge16_4
    -   -   -   -   -   -   -   -    <- gauge8_5

*/

#ifndef TM1638LedKey_h
#define TM1638LedKey_h

#include "Arduino.h"

// Команды управления яркостью
#define CMD_TM1638_DISPLAY_OFF      0x87    //дисплей выключен
#define CMD_TM1638_DISPLAY_ON       0x8F    //дисплей включен на полную яркость
// Команды управления адресацией
#define CMD_TM1638_SET_AUTOINC_ON   0x40    // режим автоинкремента адреса
#define CMD_TM1638_SET_AUTOINC_OFF  0x44    // режим указания конкретного адреса
// команда начала передачи состояния кнопок
#define CMD_TM1638_KEY_SCAN         0x42    // режим чтения кнопок (используется в getButtons)
// Адреса
#define ADR_TM1638_START            0xC0    // стартовый адрес, с которого начинается адресация СД и разрядов индикатора

const byte NUMBER_FONT[] = {
    B00111111, // 0
    B00000110, // 1
    B01011011, // 2
    B01001111, // 3
    B01100110, // 4
    B01101101, // 5
    B01111101, // 6
    B00000111, // 7
    B01111111, // 8
    B01101111, // 9
    B01110111, // A
    B01111100, // B
    B00111001, // C
    B01011110, // D
    B01111001, // E
    B01110001, // F
    B01000000, // -
    B00000000  // Clear
};
const byte DIGITS = 8;
const byte MINUS = 16;
const byte CLEAR = 17;
//------------------------------------------
// Класс для управления модулем TM1638
//------------------------------------------
class TM1638LedKey
{
private:
    uint8_t leds;
    uint8_t strobePin = 7; // строб
    uint8_t clockPin  = 9;  // часы
    uint8_t dataPin   = 8;   // данные
    // Сброс модуля
    void reset(void);
    // инициализация модуля
    void init(void);

    // Посылка команды в модуль TM1638
    void sendCommand(uint8_t command);
    // Посылка символа
    void sendSymbol(uint8_t addr, uint8_t symbol);
    // Обновляет все СД
    void setLEDs(void);
    // Отключение автоинкремента адреса
    void setAutoincOn(void);
    // Включение автоинкремента адреса
    void setAutoincOff(void);
    // Возвращает адрес СД.
    uint8_t getLEDAddress(uint8_t num);
    // Возвращает адрес разряда.
    uint8_t getGridAddress(uint8_t num);
public:
    // сейчас работает только это

    //------------------------------------------
    // Конструкторы
    //------------------------------------------
    // Пользовательское подключение модуля
    TM1638LedKey(uint8_t data, uint8_t clock, uint8_t strobe);
    // Типовое подключение модуля
    TM1638LedKey(void);

    //------------------------------------------
    // Команды
    //------------------------------------------
    /*
    *  Управление яркостью. Яркость задается и для индикатора, и для СВ. Задать отдельно нельзя.
    *    brightness - уровень яркости, может принимать следующие значения:
    *      0 - выключен
    *      1..8 - яркость
    *  Если передано число > 8, то просто включаем полную яркость
    */
    void setBrightness(uint8_t brightness);

    //------------------------------------------
    //  Светодиоды
    //------------------------------------------
    /*
    *  Установка состояния СД
    *    num - номер светодиода
    *    on  - true|false
    *  Нумерация слева направо от 1 до 8 (как на плате).
    *  Регистрируем состояние светодиода в переменной leds для последующего чтения.
    */
    void setLED(uint8_t num, uint8_t on);

    /*
     *  Получение состояния СД
     *    true  - включен
     *    false - выключен
     *  Нумерация слева направо от 1 до 8 (как на плате)
     *  Нужно учитывать, что возвращается не реальное состояние СД, а бит частной переменной leds
     */
    bool getLED(uint8_t num);

    //------------------------------------------
    // Индикаторы уровня
    //------------------------------------------

    void setGauges(uint8_t gauge1, uint8_t gauge2, uint8_t gauge3, uint8_t gauge4, uint8_t gauge5);


    void setGrid(uint8_t grid, uint8_t val, bool dp = false);


    //------------------------------------------
    //  Кнопки
    //------------------------------------------
    // TODO (spronin#1#): сейчас работает, но требует возможной оптимизации
    /*
    *  Возвращает набор битов, соотвествующий нажатым кнопкам.
    *  Нумерация кнопок слева направо (как на плате).
    *  Старший бит соотвествует кнопке 1 (слева)
    */
    uint32_t getButtons(void);
    //------------------------------------------
    // Вывод текста
    //------------------------------------------
    void clear(void);
    void showFloat(uint32_t val, uint8_t dp, uint8_t pos);
};

#endif

/* 
  Программа демонстрирует возможности модуля TM1638 
  
  Используется типовое поключение модуля:
    strobe = 7;
    clock = 9;
    data = 8;
*/

#include "TM1638LedKey.h"

uint8_t  strobe = 7;
uint8_t  clock = 9;
uint8_t  data = 8;

uint8_t demoDelay = 50;

TM1638LedKey tm(data, clock, strobe);

// СД бегут слева направо и справа налево
void LEDsdemo1(void)
{
  uint8_t i;
  for (i = 1; i <= 8; i++) {
    tm.setLED(i, 1);
    delay(demoDelay);
    tm.setLED(i, 0);
    delay(demoDelay);
  }
  for (i = 8; i >= 1; i--) {
    tm.setLED(i, 1);
    delay(demoDelay);
    tm.setLED(i, 0);
    delay(demoDelay);
  }
}  // LEDsdemo1
void LEDsdemo2(void) {
  uint8_t i;
  for (i = 1; i <= 8; i++) {
    tm.setLED(i, 1);
    getButtons();
    delay(demoDelay);
  }
  for (i = 8; i >= 1; i--) {
    tm.setLED(i, 0);
    getButtons();
    delay(demoDelay);
  }
} // LEDsdemo2

void LEDsdemo3(void) {
  uint8_t i;
  for (i = 8; i >= 1; i--) {
    tm.setLED(i, 1);
    getButtons();
    delay(demoDelay);
  }
  for (i = 1; i <= 8; i++) {
    tm.setLED(i, 0);
    getButtons();
    delay(demoDelay);
  }
} // LEDsdemo3

// Тестирование 16-разрядных индикаторов
void Gauges16(void) {
  uint8_t tmp = 0;
  for (tmp = 0; tmp <= 16; tmp++) {
    tm.setGauges(0, tmp, 0, tmp, 0);
    delay(demoDelay);
  }

  for (tmp = 16; tmp > 0; tmp--) {
    tm.setGauges(0, tmp, 0, tmp, 0);
    getButtons();
    delay(demoDelay);
  }

  // По одиночке
  for (tmp = 0; tmp <= 16; tmp++) {
    tm.setGauges(0, tmp, 0, 0, 0);
    getButtons();
    delay(demoDelay);
  }
  for (tmp = 16; tmp > 0; tmp--) {
    tm.setGauges(0, tmp, 0, 0, 0);
    getButtons();
    delay(demoDelay);
  }

  for (tmp = 0; tmp <= 16 ; tmp++) {
    tm.setGauges(0, 0, 0, tmp, 0);
    getButtons();
    delay(demoDelay);
  }

  for (tmp = 16; tmp > 0; tmp--) {
    tm.setGauges(0, 0, 0, tmp, 0);
    getButtons();
    delay(demoDelay);
  }
}  // Gauges16

// Тестирование 8-разрядных индикаторов
void Gauges8(void) {
  uint8_t tmp = 0;
  for (tmp = 0; tmp <= 8; tmp++) {
    tm.setGauges(tmp, 0, tmp, 0, tmp);
    getButtons();
    delay(demoDelay);
  }
  for (tmp = 8; tmp > 0; tmp--) {
    tm.setGauges(tmp, 0, tmp, 0, tmp);
    getButtons();
    delay(demoDelay);
  }
  // По одиночке
  for (tmp = 0; tmp <= 8; tmp++) {
    tm.setGauges(tmp, 0, 0, 0, 0);
    getButtons();
    delay(demoDelay);
  }
  for (tmp = 8; tmp > 0; tmp--) {
    tm.setGauges(tmp, 0, 0, 0, 0);
    getButtons();
    delay(demoDelay);
  }

  for (tmp = 0; tmp <= 8; tmp++) {
    tm.setGauges(0, 0, tmp, 0, 0);
    getButtons();
    delay(demoDelay);
  }

  for (tmp = 8; tmp > 0; tmp--) {
    tm.setGauges(0, 0, tmp, 0, 0);
    getButtons();
    delay(demoDelay);
  }

  for (tmp = 0; tmp <= 8; tmp++) {
    tm.setGauges(0, 0, 0, 0, tmp);
    delay(demoDelay);
  }
  for (tmp = 8; tmp > 0; tmp--) {
    tm.setGauges(0, 0, 0, 0, tmp);
    delay(demoDelay);
    getButtons();
  }
} // Gauges8

void Font() {
  for (uint8_t t = 1; t <= 8; t++) {
    delay(demoDelay);
    for (uint8_t n = 0; n < 15; n++) {
      tm.setGrid(t, n);
      delay(demoDelay);
      getButtons();
    }
  }
}  // Font

void getButtons(void) {
  uint32_t keys = 0;
  uint8_t counter;
  keys = tm.getButtons();
  for (uint8_t c = 0; c < 8; c++) {
    if (bitRead(keys, c)) {
      tm.setLED(c, !tm.getLED(c));
    }
  }
} // getButtons

double GetTemp(void)
{
  unsigned int wADC;
  double t;
  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.
  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA, ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - 324.31 ) / 1.22;

  // The returned temperature is in degrees Celsius.
  return (t);
}
//
//  Выполняется один раз при инициализации
//
void setup() {
  // Устанавливаем яркость
  tm.setBrightness(3);
  Serial.begin(9600);
}
//
//  Выполняется в цикле
//
void loop( ) {
  tm.clear();
  double t = GetTemp();
  unsigned char pos = 0;
  for (pos == 1; pos <= 5; pos++) {
    tm.showFloat(t * 100, 2, pos);
    delay(500);
    tm.clear();
  }
  LEDsdemo1();
  LEDsdemo2();
  LEDsdemo3();
  Gauges8();
  Gauges16();
  Font();
}


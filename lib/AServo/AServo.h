#ifndef ASERVO_H
#define ASERVO_H

#define F_CPU       8000000

#include <avr/io.h>
#include <avr/interrupt.h>

///Максимальное количество сервоприводов
#ifndef SERVOS_MAX
#define SERVOS_MAX 5
#endif

///Управление сервоприводом для AVR
class AServo {
public:
  ///Подключение сервопривода
  AServo(uint16_t time_us_min, uint16_t time_us_max);
  ///Подключение сервопривода к порту PORT и пину PIN
  void attach(volatile uint8_t *PORT, uint8_t PIN);
  ///Поворот сервопривода на угол angle
  void write(uint8_t angle);
  ///Задержка для минимального положения
  uint16_t timeMin;
  ///Задержка для максимального положения
  uint16_t timeMax;
  ///Текущая задержка
  uint16_t timeCurrent;
  ///Порт сервопривода
  volatile uint8_t *port;
  ///Пин сервопривода
  uint8_t pin;
};

#endif

/* Автор: Вдовин Владислав (@vladislick) */

#define F_CPU       8000000
#define SERVOS_MAX  4

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/* Подключаем библиотеки для работы с оборудованием */
#include "AServo.h"
#include "ASerial.h"
#include "AStepEngine.h"

/* Делаем управление светодиодом более удобным */
#define   LED_INIT()  DDRB |= (1 << 7)
#define   LED_OFF()   PORTB &= ~(1 << 7)
#define   LED_ON()    PORTB |= (1 << 7)

/* Максимальное количество шагов */
#define   ENGINEX_MAX 240
#define   ENGINEY_MAX 160

/* Создаём все объекты для работы оборудования */
ASerial       serial(true);       //Последовательный порт
AServo        servo(800, 2600);   //Cервопривод
AStepEngine   engineX(BIPOLAR);   //Первый шаговый двигатель
AStepEngine   engineY(BIPOLAR);   //Второй шаговый двигатель

/* Возвращает модуль числа num */
int16_t abs(int16_t num) {
  if (num < 0) return (0 - num);
  else return num;
}

int main() {
  /* Задаём переменные для распознавания G-Code */
  char      str[30];
  char      data;
  uint8_t   index = 0;
  uint8_t   mode;
  uint16_t  value[4]; //Хранит числовые значения команд (координаты, номер команды)
  bool      past[4];

  /* Предыдущее значение сервопривода */
  uint8_t servo_last = 0;

  /* Инициализация оборудования станка */
  LED_INIT();
  serial.begin(9600);
  servo.attach(&PORTB, 6);
  engineX.attach(STANDART, &PORTD, 7, 6, 5, 4);
  engineY.attach(STANDART, &PORTC, 0, 1, 2, 3);
  engineX.setStepTime(3);
  engineY.setStepTime(6);
  engineX.setCoordinate(0);
  engineY.setCoordinate(0);

  /* Разрешаем глобалные прерывания */
  sei();

  while(1) {
    if (serial.isAvailable()) {
      data = serial.read();
      if (data == '\0' || data == '\n') {
        /* Включаем светодиод */
        LED_ON();

        /* Распознаём команду */
        past[0] = past[1] = past[2] = past[3] = false;
        for (uint8_t i = 0; i < index; i++) {
          if (str[i] == 'G') {
            mode = 0;
            value[mode] = 0;
            past[0] = true;
          } else if (str[i] == 'X') {
            mode = 1;
            value[mode] = 0;
            past[1] = true;
          } else if (str[i] == 'Y') {
            mode = 2;
            value[mode] = 0;
            past[2] = true;
          } else if (str[i] == 'Z') {
            mode = 3;
            value[mode] = 0;
            past[3] = true;
          } else if (str[i] > 47 && str[i] < 58) {
            value[mode] = (value[mode] * 10) + (str[i] - 48);
          }
        }

        /* Выполняем команду */
        if (past[0]) {
          /* Если это команда включения режима удержания позиции */
          if (value[0] == 1) {
            engineX.setPositionHolding(1);
            //engineY.setPositionHolding(1);
          }
          /* Если это команда выключения режима удержания позиции */
          else if (value[0] == 2) {
            engineX.setPositionHolding(0);
            //engineY.setPositionHolding(0);
          }

          /* Если нужно поменять текущее значение оси Z */
          if (past[3]) {
            servo.write(value[3]);

            if (abs(servo_last - value[3]) < 40)
              _delay_ms(120);
            else
              _delay_ms(450);

            servo_last = value[3];
          }
          /* Если нужно поменять текущее значение оси X */
          if (past[1]) if (value[1] <= ENGINEX_MAX) engineX.move(value[1]);
          /* Если нужно поменять текущее значение оси Y */
          if (past[2]) if (value[2] <= ENGINEY_MAX) engineY.move(value[2]);
        }

        serial.write('y');
        LED_OFF();

        index = 0;
      } else {
        str[index] = data;
        index++;
      }
    }
  }
}

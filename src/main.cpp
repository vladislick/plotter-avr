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

/* Делаем управление зелёным светодиодом более удобным */
#define   LED_GREEN_INIT()  DDRC |= (1 << 4)
#define   LED_GREEN_OFF()   PORTC &= ~(1 << 4)
#define   LED_GREEN_ON()    PORTC |= (1 << 4)
#define   LED_GREEN_IS_ON() PORTC & (1 << 4)

/* Делаем управление жёлтым светодиодом более удобным */
#define   LED_YELLOW_INIT()  DDRC |= (1 << 5)
#define   LED_YELLOW_OFF()   PORTC &= ~(1 << 5)
#define   LED_YELLOW_ON()    PORTC |= (1 << 5)

/* Максимальное количество шагов */
#define   ENGINEX_MAX 240
#define   ENGINEY_MAX 160

/* Создаём все объекты для работы оборудования */
ASerial       serial(true);       //Последовательный порт
AServo        servo(600, 2800);   //Cервопривод
AStepEngine   engineX(BIPOLAR);   //Первый шаговый двигатель
AStepEngine   engineY(BIPOLAR);   //Второй шаговый двигатель

volatile char led_index1, //Для увеличения задержки таймера 0
              led_index2; //Для увеличения задержки таймера 2

/* Возвращает модуль числа num */
inline int16_t abs(int16_t num) {
  if (num < 0) return (0 - num);
  else return num;
}

/* Выключаем жёлтый светодиод по таймеру */
ISR(TIMER0_OVF_vect) {
  if (led_index1 > 3) {
    LED_YELLOW_OFF();
    TCCR0 = 0;
  } else led_index1++;
}

/* Мигаем зелёным светодиодом по таймеру */
ISR(TIMER2_OVF_vect) {
  if (led_index2 > 30) {

    if (LED_GREEN_IS_ON()) {
      LED_GREEN_OFF();
    } else {
      LED_GREEN_ON();
    }

    led_index2 = 0;
  } else led_index2++;
}

int main() {
  /* Задаём переменные для распознавания G-Code */
  char      str[40];
  char      data;
  uint8_t   index = 0;
  uint8_t   mode;
  uint16_t  value[5]; //Хранит числовые значения команд (координаты, номер команды)
  bool      past[5];

  /* Предыдущее значение сервопривода */
  uint8_t servo_last = 0;

  /* Инициализация оборудования станка */
  LED_GREEN_INIT();
  LED_YELLOW_INIT();
  serial.begin(9600);
  servo.attach(&PORTB, 4);
  engineX.attach(STANDART, &PORTC, 0, 1, 2, 3);
  engineY.attach(STANDART, &PORTB, 0, 1, 2, 3);
  engineX.setStepTime(3);
  engineY.setStepTime(3);
  engineX.setCoordinate(0);
  engineY.setCoordinate(0);

  /* Настраиваем таймеры для светодиодов */
  //TCCR0 = (1 << CS02)|(1 << CS00);              //Для включения таймера 0
  //TCCR2 = (1 << CS22)|(1 << CS21)|(1 << CS20);  //Для включения таймера 2
  TCCR0 = 0;
  TCNT0 = 0;
  TCCR2 = (1 << CS22)|(1 << CS21)|(1 << CS20);
  TCNT2 = 0;
  TIMSK |= (1 << TOIE2)|(1 << TOIE0);

  /* Разрешаем глобалные прерывания */
  sei();

  while(1) {
    if (serial.isAvailable()) {

      /* Включаем светодиод */
      LED_YELLOW_ON();
      TCNT0 = 0;
      led_index1 = 0;
      TCCR0 = (1 << CS02)|(1 << CS00);

      /* Читаем и разбираем полученные данные */
      data = serial.read();
      if (data == '\0' || data == '\n') {
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
          } else if (str[i] == 'F') {
            mode = 4;
            value[mode] = 0;
            past[4] = true;
          } else if (str[i] > 47 && str[i] < 58) {
            value[mode] = (value[mode] * 10) + (str[i] - 48);
          }
        }

        /* Выполняем команду */
        if (past[0]) {

          /* Включаем на постоянно зелёный светодиод */
          TCCR2 = 0;
          TCNT2 = 0;
          led_index2 = 0;
          LED_GREEN_ON();

          /* Если это команда включения режима удержания позиции */
          if (value[0] == 1) {
            //engineX.setPositionHolding(1);
            engineY.setPositionHolding(1);
          }
          /* Если это команда выключения режима удержания позиции */
          else if (value[0] == 2) {
            //engineX.setPositionHolding(0);
            engineY.setPositionHolding(0);
          }

          /* Если нужно изменить скорость */
          if (past[4] && value[4] > 0 && value[4] < 11) {
            engineX.setStepTime(12 - value[4]);
            engineY.setStepTime(12 - value[4]);
          }
          /* Если нужно поменять текущее значение оси Z */
          if (past[3]) {
            servo.write(value[3]);
            _delay_ms_fix(260 + engineX.getStepTime() * 20);
          }
          /* Если нужно поменять текущее значение оси X */
          if (past[1]) if (value[1] <= ENGINEX_MAX) {
            engineX.move(value[1]);
            _delay_ms(10);
          }
          /* Если нужно поменять текущее значение оси Y */
          if (past[2]) if (value[2] <= ENGINEY_MAX) {
            engineY.move(value[2]);
            _delay_ms(10);
          }
        }

        serial.write('y');

        index = 0;

        /* Возвращаем зелёный светодиод обратно в мигающее состояние */
        TCCR2 = (1 << CS22)|(1 << CS21)|(1 << CS20);
      } else {
        str[index] = data;
        index++;
      }
    }
  }
}

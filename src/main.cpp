/* Автор: Вдовин Владислав (@vladislick) */

#define F_CPU       8000000
#define STEP_DELAY  4
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

/* Создаём все объекты для работы оборудования */
ASerial       serial(true);       //Последовательный порт
AServo        servo(800, 2700);  //Cервопривод
AStepEngine   engine1(BIPOLAR);   //Первый шаговый двигатель
AStepEngine   engine2(BIPOLAR);   //Второй шаговый двигатель

int main() {
  /* Инициализация оборудования станка */
  LED_INIT();
  serial.begin(9600);
  servo.attach(&PORTB, 6);
  engine1.attach(STANDART, &PORTC, 0, 1, 2, 3);
  engine2.attach(STANDART, &PORTD, 7, 6, 5, 4);

  /* Разрешаем глобалные прерывания */
  sei();

  while(1) {
    servo.write(120);
    for (uint8_t i = 0; i < 50; i++) engine1.step(FORWARD);
    for (uint8_t i = 0; i < 50; i++) engine2.step(FORWARD);
    _delay_ms(100);
    servo.write(60);
    for (uint8_t i = 0; i < 50; i++) engine1.step(BACKWARD);
    for (uint8_t i = 0; i < 50; i++) engine2.step(BACKWARD);
    _delay_ms(100);
  }
}

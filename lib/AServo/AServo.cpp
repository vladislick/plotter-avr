#include "AServo.h"

AServo *servos[SERVOS_MAX];
uint8_t servos_num = 0;
uint8_t timer_servo;

ISR(TIMER1_COMPA_vect) {
  //Подаём питание на все сервоприводы
  for (uint8_t i = 0; i < servos_num; i++) *servos[i]->port |= (1 << servos[i]->pin);

  //Сортируем массив сервоприводов по времени
  bool completed;
  do {
    completed = 1;
    for (uint8_t i = 0; i < (servos_num - 1); i++) {
      if (servos[i]->timeCurrent > servos[i + 1]->timeCurrent) {
        AServo *srv_tmp = servos[i];
        servos[i] = servos[i + 1];
        servos[i + 1] = srv_tmp;
        completed = 0;
      }
    }
  } while(!completed);

  OCR1B = F_CPU/64000 * servos[0]->timeCurrent / 1000;
  timer_servo = 0;
}

ISR(TIMER1_COMPB_vect) {
  //Отключаем пин сервопривода(-ов)
  do {
    *servos[timer_servo]->port &= ~(1 << servos[timer_servo]->pin);
    if (timer_servo == (servos_num - 1)) break;
    timer_servo++;
  } while(servos[timer_servo - 1]->timeCurrent == servos[timer_servo]->timeCurrent);

  //Задаём значение таймера для следующего сервопривода
  if (timer_servo != (servos_num - 1)) {
    OCR1B = F_CPU/64000 * servos[timer_servo]->timeCurrent / 1000;
  }
}

AServo::AServo(uint16_t time_us_min, uint16_t time_us_max) {
  timeMin = time_us_min;
  timeMax = time_us_max;
  timeCurrent = time_us_min;

  servos_num++;
  servos[servos_num - 1] = this;
}

void AServo::attach(volatile uint8_t *PORT, uint8_t PIN) {
  port = PORT;
  pin = PIN;
  *(port - 1) |= (1 << pin);
  TCCR1B = (1 << WGM12)|(1 << CS11)|(1 << CS10);
  OCR1A = F_CPU/64/50; //Делаем таймер на 50Гц
  TIMSK |= (1 << OCIE1A)|(1 << OCIE1B);
}

void AServo::write(uint8_t angle) {
  if (angle > 180) return;
  timeCurrent = (long)(timeMax - timeMin) * (long)angle / 180;
  timeCurrent += timeMin;
}

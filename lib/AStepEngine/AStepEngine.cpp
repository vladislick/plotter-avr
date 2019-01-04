#include "AStepEngine.h"

///Делает паузу в delay мс (работает с переменными)
inline void _delay_ms_fix(uint8_t delay) {
  while (delay > 0) {
    _delay_ms(1);
    delay--;
  }
}

AStepEngine::AStepEngine(EngineType type) {
  engineType = type;
}

///Устанавливает время задержки шага
void AStepEngine::setStepTime(uint8_t time) {
  stepTime = time;
}

///Подключение двигателя
void AStepEngine::attach(EngineMode MODE, volatile uint8_t *PORT, uint8_t A1, uint8_t A2, uint8_t B1, uint8_t B2) {
  /* Записываем данные о двигателе */
  port        = PORT;
  pins[0]     = A1;
  pins[1]     = A2;
  pins[2]     = B1;
  pins[3]     = B2;
  engineMode  = MODE;
  currentStep = 0;
  stepTime = 5;

  /* Настраиваем пины */
  for (uint8_t i = 0; i < 4; i++) {
    *(port - 1) |= (1 << pins[i]);
    *port &= ~(1 << pins[i]);
  }
  /* Делаем первый шаг */
  *port |= (1 << pins[0]);
  _delay_ms_fix(stepTime);
  *port &= ~(1 << pins[0]);
}

///Сделать шаг двигателя в направлении direction
void AStepEngine::step(EngineDir direction) {
  uint8_t d;
  /* ------- ПОЛНОШАГОВЫЙ РЕЖИМ ------- */
  if (engineMode == STANDART) {
    /* Переключаем текущий шаг */
    if (direction == FORWARD) {
      if (currentStep < 3) currentStep++;
      else currentStep = 0;
    } else {
      if (currentStep > 0) currentStep--;
      else currentStep = 3;
    }
    /* Даём d реальный номер пина */
    if (engineType == BIPOLAR) {
      if (currentStep == 0) d = 0;
      else if (currentStep == 1) d = 2;
      else if (currentStep == 2) d = 1;
      else if (currentStep == 3) d = 3;
    }
    else d = currentStep;
    /* Делаем шаг */
    *port |= (1 << pins[d]);
    _delay_ms_fix(stepTime);
    *port &= ~(1 << pins[d]);
  }
  /* ------- ПОЛУШАГОВЫЙ РЕЖИМ ------- */
  else if (engineMode == HALF) {
    /* Переключаем текущий шаг */
    if (direction == FORWARD) {
      if (currentStep < 7) currentStep++;
      else currentStep = 0;
    } else {
      if (currentStep > 0) currentStep--;
      else currentStep = 7;
    }
    /* Даём d реальные номера пинов */
    if (currentStep == 0)       d = 0b0001;
    else if (currentStep == 1)  d = 0b0011;
    else if (currentStep == 2)  d = 0b0010;
    else if (currentStep == 3)  d = 0b0110;
    else if (currentStep == 4)  d = 0b0100;
    else if (currentStep == 5)  d = 0b1100;
    else if (currentStep == 6)  d = 0b1000;
    else if (currentStep == 7)  d = 0b1001;

    /* Включаем нужные пины */
    for (uint8_t i = 0; i < 4; i++) {
      if (d & (1 << i)) {
        if (i == 0) *port |= (1 << pins[0]);
        else if (i == 1) *port |= (1 << pins[2]);
        else if (i == 2) *port |= (1 << pins[1]);
        else if (i == 3) *port |= (1 << pins[3]);
      }
    }

    /* Выключаем пины */
    _delay_ms_fix(stepTime);
    for (uint8_t i = 0; i < 4; i++) *port &= ~(1 << pins[i]);
  }
}

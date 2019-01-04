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
    /* Даём i реальный номер пина */
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
    if (direction == FORWARD) {

    } else {

    }
  }
}

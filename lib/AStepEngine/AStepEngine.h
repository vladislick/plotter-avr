#ifndef ASTEPENGINE_H
#define ASTEPENGINE_H

#define F_CPU       8000000

#ifndef STEP_DELAY
#define STEP_DELAY  4
#endif

#include <avr/io.h>
#include <util/delay.h>

///Тип двигателя: биполярный, униполярный
enum EngineType { BIPOLAR, UNIPOLAR };
///Режим работы двигателя: полный шаг, полушаг
enum EngineMode { STANDART, HALF };
///Направление шага: вперёд, назад
enum EngineDir { FORWARD, BACKWARD};

///Последовательный порт
class AStepEngine {
public:
  AStepEngine(EngineType type);
  ///Подключение двигателя
  void attach(EngineMode MODE, volatile uint8_t *PORT, uint8_t A1, uint8_t A2, uint8_t B1, uint8_t B2);
  ///Сделать шаг двигателя в направлении direction
  void step(EngineDir direction);
private:
  ///Тип шагового двигателя
  EngineType engineType;
  ///Режим работы двигателя
  EngineMode engineMode;
  ///Порт
  volatile uint8_t *port;
  ///Пины
  uint8_t pins[4];
  ///Номер текущего шага
  volatile uint8_t currentStep;
};

#endif

#ifndef ASERIAL_H
#define ASERIAL_H

#define F_CPU       8000000

#include <avr/io.h>

///Последовательный порт
class ASerial {
public:
  ///Последовательный порт (указать скорость)
  ASerial(bool);
  ///Инициализация
  void begin(short baud_rate);
  ///Есть ли принятые данные
  bool isAvailable();
  ///Прочитать байт
  char read();
  ///Отправить байт
  void write(char);
  ///Отправить строку
  void write(const char*);
};

#endif

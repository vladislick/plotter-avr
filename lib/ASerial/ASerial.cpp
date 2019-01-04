#include "ASerial.h"

ASerial::ASerial(bool a) {
  a += 1;
}

void ASerial::begin(short baud_rate) {
  UCSRB |= (1 << RXEN)|(1 << TXEN);
  UCSRC |= (1 << URSEL)|(1 << UCSZ1)|(1 << UCSZ0);
  UBRRL = F_CPU/((long)baud_rate * (long)16) - 1;
}

bool ASerial::isAvailable() {
  return (UCSRA & (1 << RXC));
}

char ASerial::read() {
  return UDR;
}

void ASerial::write(char data_byte) {
  UDR = data_byte;
}

void ASerial::write(const char *str) {
  short i = -1;
  do {
    i++;
    while(!(UCSRA & (1 << UDRE)));
    UDR = str[i];
  } while (str[i] != '\0');
}

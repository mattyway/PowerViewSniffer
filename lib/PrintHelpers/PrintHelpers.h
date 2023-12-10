#ifndef PRINTHELPERS_H
#define PRINTHELPERS_H

#include <Arduino.h>

void printHex(uint8_t num) {  //print a byte as two hex chars
  if (num < 0x10) {
    Serial.print("0");
  }
  Serial.print(num, HEX);
}

void printByteArray(const uint8_t *byteArray, const uint8_t arraySize) { //print a byte array as hex chars
  for (int i = 0; i < arraySize; i++) {
    printHex(byteArray[i]);
  }
}

void printBuffer(const uint8_t *buffer) {
  uint8_t length = buffer[1] + 4;
  printByteArray(buffer, length);
}

void printUint8(uint8_t value, bool prefix = true) {
  if (prefix) {
    Serial.print("0x");
  }
  printHex(value);
}

void printUint16(uint16_t value, bool prefix = true) {
  if (prefix) {
    Serial.print("0x");
  }
  printUint8(uint8_t(value & 0xFF00 >> 8), false);
  printUint8(uint8_t(value & 0x00FF), false);
}

#endif // PRINTHELPERS_H
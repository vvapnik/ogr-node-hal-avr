// EN_IN / EN_OUT via plain Arduino digital I/O — identical on every AVR
// target this package supports, no chip-specific branching needed.
#pragma once

#include <Arduino.h>
#include "OgrGpio.h"

namespace ogr_hal_avr {

class OgrAvrGpio : public ogr::IOgrGpio {
public:
  OgrAvrGpio(uint8_t enInPin, uint8_t enOutPin) : enInPin_(enInPin), enOutPin_(enOutPin) {}

  void begin() override {
    pinMode(enInPin_, INPUT);
    pinMode(enOutPin_, OUTPUT);
    digitalWrite(enOutPin_, LOW);
  }

  bool readEnIn() override { return digitalRead(enInPin_) == HIGH; }
  void writeEnOut(bool high) override { digitalWrite(enOutPin_, high ? HIGH : LOW); }

private:
  uint8_t enInPin_;
  uint8_t enOutPin_;
};

} // namespace ogr_hal_avr

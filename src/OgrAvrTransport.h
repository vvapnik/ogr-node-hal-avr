// I2C-slave transport for AVR targets. Two backends live in OgrAvrTransport.cpp,
// selected at compile time by which peripheral the MCU actually has — the
// class name/API is identical either way, so sketches don't change between
// ATmega328 (hardware TWI) and ATtiny85 (USI, bit-banged).
#pragma once

#include <stdint.h>
#include "OgrTransport.h"

namespace ogr_hal_avr {

class OgrAvrTransport : public ogr::IOgrTransport {
public:
  void begin(ogr::IOgrProtocolSink &sink, uint8_t address) override;
  void setAddress(uint8_t address) override;
  void end() override;
  void poll() override;

private:
  static void onReceiveTrampoline(int numBytes);
  static void onRequestTrampoline();

  static OgrAvrTransport *self_;
  ogr::IOgrProtocolSink *sink_ = nullptr;
  uint8_t lastReg_ = 0;
};

} // namespace ogr_hal_avr

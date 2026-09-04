// I2C-slave transport for AVR targets, built on Arduino's Wire.
//
// This works unchanged on both ATmega328-class chips (hardware TWI) and
// ATtiny85-class chips (USI, no hardware TWI at all): framework-arduino-avr
// and framework-arduino-avr-attiny each ship their own Wire library, picked
// automatically by which framework the current board resolves to, and both
// implement the same TwoWire API — including slave mode (`begin(address)`,
// `onReceive`, `onRequest`) — on top of whatever peripheral (TWI or USI)
// that MCU actually has. So there is nothing chip-specific left to write
// here at all.
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

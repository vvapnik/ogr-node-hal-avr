#include "OgrAvrTransport.h"
#include "OgrTypes.h" // kMaxWriteFrame / kMaxReadFrame

// Spec §5.3 (revised) never requires a node to NACK a specific byte — a node
// always ACKs and reports a bad PEC through STATUS instead. That means the
// stock Arduino Wire slave API (onReceive/onRequest, buffered) is enough on
// every AVR target this package supports; there's no need to drop to raw
// peripheral (TWCR/TWDR or USI) register handling anywhere. Which concrete
// peripheral Wire itself is built on (hardware TWI vs bit-banged USI) is
// entirely the active framework's problem — framework-arduino-avr and
// framework-arduino-avr-attiny each ship a Wire library with the same slave
// API on top of whichever one their target chip actually has.
#include <Wire.h>

namespace ogr_hal_avr {

OgrAvrTransport *OgrAvrTransport::self_ = nullptr;

void OgrAvrTransport::begin(ogr::IOgrProtocolSink &sink, uint8_t address) {
  sink_ = &sink;
  self_ = this;
  Wire.begin(address);
  Wire.onReceive(onReceiveTrampoline);
  Wire.onRequest(onRequestTrampoline);
}

void OgrAvrTransport::setAddress(uint8_t address) {
  // Re-arming the callbacks is defensive, not strictly required — Wire.begin()
  // reinitializes the peripheral but doesn't drop the previously-registered
  // handlers on either implementation.
  Wire.begin(address);
  Wire.onReceive(onReceiveTrampoline);
  Wire.onRequest(onRequestTrampoline);
}

void OgrAvrTransport::end() { Wire.end(); }

void OgrAvrTransport::poll() {
  // Fully interrupt-driven on both TWI and USI Wire implementations —
  // nothing to pump here.
}

void OgrAvrTransport::onReceiveTrampoline(int numBytes) {
  if (!self_ || numBytes <= 0) {
    return;
  }

  uint8_t buf[ogr::kMaxWriteFrame];
  uint8_t n = 0;
  while (Wire.available() && n < sizeof(buf)) {
    buf[n++] = static_cast<uint8_t>(Wire.read());
  }
  while (Wire.available()) {
    Wire.read(); // defensive: drain anything past our max frame size
  }
  if (n == 0) {
    return;
  }

  self_->lastReg_ = buf[0];

  // A length of exactly 1 is always just the register-select prelude ahead
  // of a read (repeated START) — no writable register on this protocol
  // accepts a 0-byte payload, so it can never be a genuine complete write.
  if (n >= 2) {
    self_->sink_->onI2cWrite(buf, n);
  }
}

void OgrAvrTransport::onRequestTrampoline() {
  if (!self_) {
    return;
  }
  uint8_t resp[ogr::kMaxReadFrame];
  uint8_t n = self_->sink_->onI2cRead(self_->lastReg_, resp, sizeof(resp));
  Wire.write(resp, n);
}

} // namespace ogr_hal_avr

// PLANT_UID persistence via the AVR's built-in EEPROM.
#pragma once

#include <EEPROM.h>
#include "OgrStorage.h"

namespace ogr_hal_avr {

class OgrAvrEepromStorage : public ogr::IOgrStorage {
public:
  // eepromAddress: byte offset to store the 4-byte UID at. Change it if
  // something else on the same MCU also uses EEPROM and would overlap.
  explicit OgrAvrEepromStorage(int eepromAddress = 0) : address_(eepromAddress) {}

  bool loadPlantUid(uint32_t &uid) override {
    uint32_t stored;
    EEPROM.get(address_, stored);
    // An AVR EEPROM cell that was never written reads back as 0xFF — which
    // is bit-for-bit ogr::kNoPlantUid, so "untouched" and "explicitly
    // cleared" read back identically with no first-boot special case.
    if (stored == 0xFFFFFFFFul) {
      return false;
    }
    uid = stored;
    return true;
  }

  void savePlantUid(uint32_t uid) override {
    EEPROM.put(address_, uid); // only rewrites bytes that actually changed
  }

private:
  int address_;
};

} // namespace ogr_hal_avr

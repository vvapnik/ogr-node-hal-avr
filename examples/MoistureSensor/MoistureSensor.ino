// Plant Container module (spec CLASS 0x01): a soil moisture sensor plus a
// water valve, on an ATmega328 (Arduino Uno/Nano). Wire EN_IN/EN_OUT to the
// module's bottom/top connector per docs/02-connector.md and
// docs/03-enumeration.md in the open-garden-rack spec.
//
// This sketch is the entire hardware-facing surface a user is expected to
// write: declare endpoints, attach callbacks, call begin()/update(). Nothing
// here touches I2C registers or the enumeration state machine directly.
#include <OgrNodeAvr.h>

using namespace ogr;
using namespace ogr_hal_avr;

constexpr uint8_t kEnInPin = 2;
constexpr uint8_t kEnOutPin = 3;
constexpr uint8_t kMoisturePin = A0;
constexpr uint8_t kValvePin = 4;

OgrNode node(ClassId::PlantContainer, /*isAnchor=*/true);

OgrAvrTransport transport;
OgrAvrGpio gpio(kEnInPin, kEnOutPin);
OgrAvrEepromStorage storage;

uint32_t readMoisture(void *) {
  return analogRead(kMoisturePin); // 0-1023 raw ADC, per TypeId::Moisture (spec §7)
}

void writeValve(void *, uint32_t value) {
  digitalWrite(kValvePin, value ? HIGH : LOW);
}

void onPlantAssigned(uint32_t uid) {
  // Optional: react to a new plant being assigned to this container, e.g.
  // reset a local watering schedule. Safe to leave unimplemented.
  (void)uid;
}

void setup() {
  pinMode(kValvePin, OUTPUT);
  digitalWrite(kValvePin, LOW);

  node.addEndpoint(TypeId::Moisture, Mode::Read, Format::Raw16, readMoisture);
  node.addEndpoint(TypeId::WaterValve, Mode::Write, Format::Binary, nullptr, writeValve);
  node.onPlantUidChanged(onPlantAssigned);

  node.begin(transport, gpio, storage); // storage present -> PLANT_UID persists (mandatory for anchor modules)
}

void loop() {
  node.update();
}

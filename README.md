# ogr-node-hal-avr

AVR hardware abstraction layer for [ogr-node-core](https://github.com/vvapnik/ogr-node-core)
— the platform-independent [OpenGardenRack](https://github.com/vvapnik/open-garden-rack)
node protocol engine. Install both libraries; this one supplies the
`IOgrTransport` / `IOgrGpio` / `IOgrStorage` implementations that let
`ogr-node-core` actually run on an AVR MCU.

| Target | Peripheral | Status |
|---|---|---|
| ATmega328 (Arduino Uno/Nano/Pro Mini, ...) | Hardware TWI, via Arduino `Wire` | Supported |
| ATtiny85 | USI, bit-banged | Planned, not yet implemented |

The transport backend is selected at compile time by which register set the
target MCU actually has (`TWCR` vs `USIDR`) — sketches use the same
`OgrAvrTransport` class name either way.

## Usage

```cpp
#include <OgrNodeAvr.h>

using namespace ogr;
using namespace ogr_hal_avr;

OgrNode node(ClassId::PlantContainer, /*isAnchor=*/true);
OgrAvrTransport transport;
OgrAvrGpio gpio(/*enIn=*/2, /*enOut=*/3);
OgrAvrEepromStorage storage;

uint32_t readMoisture(void *) { return analogRead(A0); }

void setup() {
  node.addEndpoint(TypeId::Moisture, Mode::Read, Format::Raw16, readMoisture);
  node.begin(transport, gpio, storage);
}

void loop() {
  node.update();
}
```

See [examples/MoistureSensor](examples/MoistureSensor) for a complete
sketch (moisture sensor + water valve, PLANT_UID persisted to EEPROM).

## Why plain `Wire`, not a raw TWCR/TWDR driver?

The (revised) OpenGardenRack spec §5.3 never requires a node to NACK a
specific byte — a node always ACKs a write and reports a bad PEC through
`STATUS` instead. That removes the one requirement that would have forced a
low-level, per-byte-ACK-control I2C slave driver, so this package builds on
Arduino's stock `Wire` slave API (`onReceive`/`onRequest`) instead of
reimplementing TWI register handling.

## Notes

- `OgrAvrGpio` and `OgrAvrEepromStorage` are chip-agnostic (plain
  `digitalRead`/`digitalWrite`/`pinMode` and the Arduino `EEPROM` library),
  so they'll work unchanged once the ATtiny85 (USI) transport backend lands.
- Endpoint `onRead`/`onWrite` callbacks run synchronously from the TWI ISR.
  Keep them fast — `digitalWrite`/`analogRead` are fine, anything that
  blocks for more than a few tens of microseconds risks stalling the I2C
  clock (see [ogr-node-core](https://github.com/vvapnik/ogr-node-core)'s
  design notes on why PLANT_UID storage writes are deliberately deferred
  out of that same call path).

## Verifying a build locally

There's no committed `platformio.ini` here (this is a library, not a
project) — use `pio ci` against an example instead:

```sh
pio ci examples/MoistureSensor/MoistureSensor.ino \
  --lib="." --lib="../ogr-node-core" --board=uno
```

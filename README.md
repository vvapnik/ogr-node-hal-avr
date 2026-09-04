# ogr-node-hal-avr

AVR hardware abstraction layer for [ogr-node-core](https://github.com/vvapnik/ogr-node-core)
— the platform-independent [OpenGardenRack](https://github.com/vvapnik/open-garden-rack)
node protocol engine. Install both libraries; this one supplies the
`IOgrTransport` / `IOgrGpio` / `IOgrStorage` implementations that let
`ogr-node-core` actually run on an AVR MCU.

| Target | Peripheral | Status |
|---|---|---|
| ATmega328 (Arduino Uno/Nano/Pro Mini, ...) | Hardware TWI, via Arduino `Wire` | Supported |
| ATtiny85 | USI, via Arduino `Wire` | Supported |

Both targets use the exact same `OgrAvrTransport` code — `Wire.h` itself
differs per framework (`framework-arduino-avr` implements it on hardware
TWI, `framework-arduino-avr-attiny` implements the identical slave API
(`begin(address)`/`onReceive`/`onRequest`) on top of USI via the bundled
USI_TWI_Slave), and PlatformIO picks whichever one matches the board. There
is no `#ifdef TWCR`/`USIDR` branching left in this package at all.

> **Board-definition gotcha:** a few PlatformIO ATtiny85 board entries (e.g.
> `trinket5`) resolve to the plain `framework-arduino-avr` core instead of
> `framework-arduino-avr-attiny`, which fails to compile *any* I2C code on
> this chip (`framework-arduino-avr`'s `Wire` assumes hardware TWI registers
> that don't exist on ATtiny85) — not specific to this library. If a board
> fails with `'TWINT' undeclared` or similar, use the generic `attiny85`
> board (or set `board_build.core = tiny` explicitly) instead.

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

## Why plain `Wire`, not a raw peripheral driver?

The (revised) OpenGardenRack spec §5.3 never requires a node to NACK a
specific byte — a node always ACKs a write and reports a bad PEC through
`STATUS` instead. That removes the one requirement that would have forced a
low-level, per-byte-ACK-control I2C slave driver (TWCR/TWDR register
handling on ATmega328, or a hand-rolled USI bit-banged state machine on
ATtiny85), so this package builds entirely on Arduino's stock `Wire` slave
API (`onReceive`/`onRequest`) — on *every* target it supports.

## Notes

- `OgrAvrGpio` and `OgrAvrEepromStorage` are chip-agnostic (plain
  `digitalRead`/`digitalWrite`/`pinMode` and the Arduino `EEPROM` library),
  so they work unchanged across every target above.
- Endpoint `onRead`/`onWrite` callbacks run synchronously from Wire's
  interrupt/ISR context on every target. Keep them fast —
  `digitalWrite`/`analogRead` are fine, anything that blocks for more than a
  few tens of microseconds risks stalling the I2C clock (see
  [ogr-node-core](https://github.com/vvapnik/ogr-node-core)'s design notes
  on why PLANT_UID storage writes are deliberately deferred out of that
  same call path). This is a real constraint on ATtiny85 specifically: it
  has no hardware clock-stretching-friendly headroom beyond what USI's
  two-wire mode itself provides.
- On ATtiny85, `Wire`'s SDA/SCL are fixed by the USI peripheral itself (USI
  DI = physical SDA, USCK = physical SCL) — there's exactly one USI, so
  unlike `OgrAvrGpio`'s EN pins, I2C pin choice isn't a sketch-level option.

## Verifying a build locally

There's no committed `platformio.ini` here (this is a library, not a
project) — use `pio ci` against an example instead:

```sh
pio ci examples/MoistureSensor/MoistureSensor.ino \
  --lib="." --lib="../ogr-node-core" --board=uno

pio ci examples/MoistureSensor/MoistureSensor.ino \
  --lib="." --lib="../ogr-node-core" --board=attiny85
```

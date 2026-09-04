// Single include for AVR sketches: pulls in the protocol engine plus this
// package's transport/GPIO/storage implementations.
#pragma once

#include <OgrNode.h>
#include "OgrAvrEepromStorage.h"
#include "OgrAvrGpio.h"
#include "OgrAvrTransport.h"

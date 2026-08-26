#include <Arduino.h>
#include <Wire.h>

#include "harpa_controller.h"
#include "lidar_array.h"

#ifndef PIO_UNIT_TESTING

LidarArray lidars;
HarpaController harpa;

namespace {

void printTransitions(const HarpaFrame& frame) {
    for (size_t index = 0; index < SENSOR_COUNT; ++index) {
        const uint16_t bit = static_cast<uint16_t>(1u << index);

        if ((frame.pressedMask & bit) != 0) {
            Serial.printf("String %u PRESSED\n", static_cast<unsigned>(index));
        }
        if ((frame.releasedMask & bit) != 0) {
            Serial.printf("String %u RELEASED\n", static_cast<unsigned>(index));
        }
    }
}

void printTelemetryIfEnabled(const LidarReadingArray& readings,
                             uint32_t nowMs) {
    if (!LidarDefaults::ENABLE_DISTANCE_TELEMETRY) {
        return;
    }

    static bool telemetryStarted = false;
    static uint32_t lastTelemetryAtMs = 0;
    if (telemetryStarted && nowMs - lastTelemetryAtMs < 100) {
        return;
    }

    telemetryStarted = true;
    lastTelemetryAtMs = nowMs;
    Serial.print("DIST");
    for (size_t index = 0; index < SENSOR_COUNT; ++index) {
        Serial.print(' ');
        if (readings[index].valid) {
            Serial.print(readings[index].distanceMm);
        } else {
            Serial.print('X');
        }
    }
    Serial.println();
}

}  // namespace

void setup() {
    Serial.begin(115200);

    const uint8_t available = lidars.begin(Wire);
    Serial.printf("Lidars available: %u/%u\n", static_cast<unsigned>(available),
                  static_cast<unsigned>(SENSOR_COUNT));
    for (size_t index = 0; index < SENSOR_COUNT; ++index) {
        const char* result = lidars.sensorAvailable(index) ? "OK" : "FAILED";
        Serial.printf("Lidar %u: %s at 0x%02X\n", static_cast<unsigned>(index),
                      result, SENSOR_CONFIGS[index].i2cAddress);
    }
}

void loop() {
    const uint32_t nowMs = millis();
    lidars.service(nowMs);
    const HarpaFrame frame = harpa.update(lidars.readings(), nowMs);
    printTransitions(frame);
    printTelemetryIfEnabled(lidars.readings(), nowMs);
}

#endif  // PIO_UNIT_TESTING

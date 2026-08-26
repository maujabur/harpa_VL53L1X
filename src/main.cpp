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
                             const HarpaFrame& frame, uint32_t nowMs) {
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
        const LidarReading& reading = readings[index];
        const uint32_t ageMs = nowMs - reading.updatedAtMs;
        const char* cacheState = !reading.valid
                                     ? "INVALID"
                                     : (ageMs > LidarDefaults::STALE_AFTER_MS
                                            ? "STALE"
                                            : "VALID");
        const uint16_t bit = static_cast<uint16_t>(1u << index);
        const char* harpState = (frame.activeMask & bit) != 0 ? "ACTIVE"
                                                              : "IDLE";

        Serial.print(static_cast<unsigned>(index));
        Serial.print(':');
        if (reading.valid) {
            Serial.print(reading.distanceMm);
        } else {
            Serial.print('X');
        }
        Serial.print(':');
        Serial.print(ageMs);
        Serial.print(':');
        Serial.print(cacheState);
        Serial.print(':');
        Serial.print(harpState);
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
                      result, static_cast<unsigned>(SENSOR_CONFIGS[index].i2cAddress));
    }
}

void loop() {
    const uint32_t nowMs = millis();
    lidars.service(nowMs);
    const HarpaFrame frame = harpa.update(lidars.readings(), nowMs);
    printTransitions(frame);
    printTelemetryIfEnabled(lidars.readings(), frame, nowMs);
}

#endif  // PIO_UNIT_TESTING

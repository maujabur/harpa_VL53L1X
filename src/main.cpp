#include <Arduino.h>
#include <Keyboard.h>
#include <Wire.h>

#include "harpa_controller.h"
#include "harpa_keymap.h"
#include "lidar_array.h"

#ifndef PIO_UNIT_TESTING

LidarArray lidars;
HarpaController harpa;

namespace {

void printTransitions(const HarpaFrame& frame) {
    for (size_t index = 0; index < SENSOR_COUNT; ++index) {
        const uint16_t bit = static_cast<uint16_t>(1u << index);

        if ((frame.pressedMask & bit) != 0) {
            Serial.print(F("String "));
            Serial.print(static_cast<unsigned>(index));
            Serial.println(F(" PRESSED"));
        }
        if ((frame.releasedMask & bit) != 0) {
            Serial.print(F("String "));
            Serial.print(static_cast<unsigned>(index));
            Serial.println(F(" RELEASED"));
        }
    }
}

void emitKeystrokes(const HarpaFrame& frame) {
    for (size_t index = 0; index < SENSOR_COUNT; ++index) {
        const uint16_t bit = static_cast<uint16_t>(1u << index);

        if ((frame.pressedMask & bit) != 0) {
            Keyboard.press(HarpaKeymap::keyForSensor(index));
        }
        if ((frame.releasedMask & bit) != 0) {
            Keyboard.release(HarpaKeymap::keyForSensor(index));
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
    Keyboard.begin();

    const uint8_t available = lidars.begin(Wire);
    Serial.print(F("Lidars available: "));
    Serial.print(static_cast<unsigned>(available));
    Serial.print('/');
    Serial.println(static_cast<unsigned>(SENSOR_COUNT));
    for (size_t index = 0; index < SENSOR_COUNT; ++index) {
        const __FlashStringHelper* result =
            lidars.sensorAvailable(index) ? F("OK") : F("FAILED");
        Serial.print(F("Lidar "));
        Serial.print(static_cast<unsigned>(index));
        Serial.print(F(": "));
        Serial.print(result);
        Serial.print(F(" at 0x"));
        Serial.println(static_cast<unsigned>(SENSOR_CONFIGS[index].i2cAddress),
                       HEX);
    }
}

void loop() {
    const uint32_t nowMs = millis();
    lidars.service(nowMs);
    const HarpaFrame frame = harpa.update(lidars.readings(), nowMs);
    emitKeystrokes(frame);
    printTransitions(frame);
    printTelemetryIfEnabled(lidars.readings(), frame, nowMs);
}

#endif  // PIO_UNIT_TESTING

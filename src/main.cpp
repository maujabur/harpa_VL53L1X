#include <Arduino.h>
#include <VL53L1X.h>
#include <Wire.h>

#include "measurement_status.h"
#include "lidar_config.h"

#ifndef PIO_UNIT_TESTING

namespace {
VL53L1X sensor;
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(1000);

    Wire.begin(LidarDefaults::SDA_PIN, LidarDefaults::SCL_PIN);
    Wire.setClock(LidarDefaults::I2C_CLOCK_HZ);

    const LidarConfig& config = SENSOR_CONFIGS[0];
    sensor.setTimeout(LidarDefaults::SENSOR_TIMEOUT_MS);
    if (!sensor.init()) {
        Serial.println("ERRO: VL53L1X nao foi detectado no endereco 0x29.");
        while (true) {
            delay(1000);
        }
    }

    sensor.setDistanceMode(VL53L1X::Short);
    sensor.setMeasurementTimingBudget(LidarDefaults::MEASUREMENT_BUDGET_US);
    sensor.setROISize(config.roi.width, config.roi.height);
    sensor.setROICenter(
        spadNumberFromCoordinates(config.roi.centerX, config.roi.centerY));
    sensor.startContinuous(LidarDefaults::MEASUREMENT_PERIOD_MS);

    Serial.printf("ROI: %ux%u, centro=(%u,%u), SPAD=%u\n",
                  config.roi.width, config.roi.height, config.roi.centerX,
                  config.roi.centerY,
                  spadNumberFromCoordinates(config.roi.centerX,
                                            config.roi.centerY));
    Serial.println("VL53L1X iniciado. Publicando distancias em milimetros:");
}

void loop() {
    const uint16_t distanceMm = sensor.read();
    const MeasurementStatus status =
        classifyMeasurement(sensor.timeoutOccurred());

    if (status == MeasurementStatus::TimedOut) {
        Serial.println("ERRO: timeout na leitura.");
        return;
    }

    Serial.printf("Distancia: %u mm\n", distanceMm);
}

#endif  // PIO_UNIT_TESTING

#include <Arduino.h>
#include <VL53L1X.h>
#include <Wire.h>

#include "measurement_status.h"
#include "sensor_config.h"

namespace {
constexpr uint8_t SDA_PIN = 21;
constexpr uint8_t SCL_PIN = 22;
constexpr uint32_t I2C_CLOCK_HZ = 400000;
constexpr uint16_t SENSOR_TIMEOUT_MS = 500;
constexpr uint32_t MEASUREMENT_BUDGET_US = 50000;
constexpr uint32_t MEASUREMENT_PERIOD_MS = 100;

VL53L1X sensor;
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(1000);

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(I2C_CLOCK_HZ);

    sensor.setTimeout(SENSOR_TIMEOUT_MS);
    if (!sensor.init()) {
        Serial.println("ERRO: VL53L1X nao foi detectado no endereco 0x29.");
        while (true) {
            delay(1000);
        }
    }

    sensor.setDistanceMode(VL53L1X::Long);
    sensor.setMeasurementTimingBudget(MEASUREMENT_BUDGET_US);
    sensor.setROISize(SensorConfig::ROI_WIDTH, SensorConfig::ROI_HEIGHT);
    sensor.setROICenter(ROI_CENTER_SPAD);
    sensor.startContinuous(MEASUREMENT_PERIOD_MS);

    Serial.printf("ROI: %ux%u, centro=(%u,%u), SPAD=%u\n",
                  SensorConfig::ROI_WIDTH, SensorConfig::ROI_HEIGHT,
                  SensorConfig::ROI_CENTER_X, SensorConfig::ROI_CENTER_Y,
                  ROI_CENTER_SPAD);
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

#include "lidar_array.h"

#include <Arduino.h>

uint8_t LidarArray::begin(TwoWire& bus) {
    readings_ = {};
    nextSensor_ = 0;
    availableCount_ = 0;

    for (size_t index = 0; index < SENSOR_COUNT; ++index) {
        sensors_[index].holdInReset(SENSOR_CONFIGS[index]);
    }

    delay(2);
    bus.begin(LidarDefaults::SDA_PIN, LidarDefaults::SCL_PIN,
              LidarDefaults::I2C_CLOCK_HZ);

    for (size_t index = 0; index < SENSOR_COUNT; ++index) {
        if (sensors_[index].begin(bus)) {
            ++availableCount_;
        }
    }

    return availableCount_;
}

void LidarArray::service(uint32_t nowMs) {
    sensors_[nextSensor_].service(nowMs);
    readings_[nextSensor_] = sensors_[nextSensor_].reading();
    nextSensor_ = (nextSensor_ + 1) % SENSOR_COUNT;
}

const LidarReadingArray& LidarArray::readings() const { return readings_; }

uint8_t LidarArray::availableCount() const { return availableCount_; }

bool LidarArray::sensorAvailable(size_t index) const {
    return index < SENSOR_COUNT && sensors_[index].available();
}

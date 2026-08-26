#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <Wire.h>

#include "lidar_reading.h"
#include "lidar_sensor.h"

class LidarArray {
   public:
    uint8_t begin(TwoWire& bus);
    void service(uint32_t nowMs);
    const LidarReadingArray& readings() const;
    uint8_t availableCount() const;
    bool sensorAvailable(size_t index) const;

   private:
    std::array<LidarSensor, SENSOR_COUNT> sensors_{};
    LidarReadingArray readings_{};
    size_t nextSensor_ = 0;
    uint8_t availableCount_ = 0;
};

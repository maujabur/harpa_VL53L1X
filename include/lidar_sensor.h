#pragma once

#include <VL53L1X.h>
#include <Wire.h>

#include "harpa_array.h"

#include "lidar_config.h"
#include "lidar_reading.h"

class LidarSensor {
   public:
    void holdInReset(const LidarConfig& config);
    bool begin(TwoWire& bus);
    void service(uint32_t nowMs);
    const LidarReading& reading() const;
    bool available() const;

   private:
    void failAndReset();

    LidarConfig config_{};
    VL53L1X driver_{};
    LidarReading reading_{};
    bool available_ = false;
};

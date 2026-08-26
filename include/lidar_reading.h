#pragma once

#include <array>
#include <cstdint>

#include "lidar_config.h"

struct LidarReading {
    uint16_t distanceMm = 0;
    uint32_t updatedAtMs = 0;
    bool valid = false;
};

using LidarReadingArray = std::array<LidarReading, SENSOR_COUNT>;

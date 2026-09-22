#pragma once

#include "harpa_array.h"
#include "lidar_config.h"

struct LidarReading {
    uint16_t distanceMm = 0;
    uint32_t updatedAtMs = 0;
    bool valid = false;
};

using LidarReadingArray = harpa_stl::Array<LidarReading, SENSOR_COUNT>;

#pragma once

#include "harpa_array.h"
#include "lidar_reading.h"

struct HarpaFrame {
    uint16_t activeMask = 0;
    uint16_t pressedMask = 0;
    uint16_t releasedMask = 0;
};

class HarpaController {
   public:
    HarpaFrame update(const LidarReadingArray& readings, uint32_t nowMs);

   private:
    uint16_t activeMask_ = 0;
};

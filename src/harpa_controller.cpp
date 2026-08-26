#include "harpa_controller.h"

HarpaFrame HarpaController::update(const LidarReadingArray& readings,
                                   uint32_t nowMs) {
    HarpaFrame frame{};

    for (size_t index = 0; index < SENSOR_COUNT; ++index) {
        const uint16_t bit = static_cast<uint16_t>(1u << index);
        const LidarReading& reading = readings[index];
        const bool fresh = nowMs - reading.updatedAtMs <=
                           LidarDefaults::STALE_AFTER_MS;
        const bool canTrigger = reading.valid && fresh;
        const bool active = (activeMask_ & bit) != 0;

        if (!active && canTrigger &&
            reading.distanceMm < SENSOR_CONFIGS[index].triggerMm) {
            activeMask_ |= bit;
            frame.pressedMask |= bit;
        } else if (active &&
                   (!canTrigger ||
                    reading.distanceMm > SENSOR_CONFIGS[index].releaseMm)) {
            activeMask_ &= static_cast<uint16_t>(~bit);
            frame.releasedMask |= bit;
        }
    }

    frame.activeMask = activeMask_;
    return frame;
}

#include <Arduino.h>

#include <type_traits>

#include "lidar_sensor.h"

static_assert(std::is_default_constructible<LidarSensor>::value);

[[maybe_unused]] void repeatedBootLifecycleCompiles(LidarSensor& sensor,
                                                    TwoWire& bus) {
    sensor.holdInReset(SENSOR_CONFIGS[0]);
    (void)sensor.begin(bus);
    sensor.holdInReset(SENSOR_CONFIGS[0]);
    (void)sensor.begin(bus);
}

void setup() {
    LidarSensor sensor;
    sensor.holdInReset(SENSOR_CONFIGS[0]);
    (void)sensor.available();
    (void)sensor.reading();
}

void loop() {}

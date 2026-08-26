#include <Arduino.h>
#include "lidar_config.h"

static_assert(SENSOR_COUNT == 12, "A harpa deve ter 12 sensores");
static_assert(spadNumberFromCoordinates(0, 0) == 128);
static_assert(spadNumberFromCoordinates(15, 15) == 0);
static_assert(spadNumberFromCoordinates(8, 7) == 199);
static_assert(isValidRoi({4, 4, 8, 7}));
static_assert(!isValidRoi({3, 4, 8, 7}));
static_assert(configsAreValid(SENSOR_CONFIGS));
static_assert(SENSOR_CONFIGS[0].xshutPin == 4);
static_assert(SENSOR_CONFIGS[0].i2cAddress == 0x30);
static_assert(LidarDefaults::SDA_PIN == 21);
static_assert(LidarDefaults::SCL_PIN == 22);
static_assert(LidarDefaults::I2C_CLOCK_HZ == 400000);
static_assert(LidarDefaults::MEASUREMENT_BUDGET_US == 20000);
static_assert(LidarDefaults::MEASUREMENT_PERIOD_MS == 25);
static_assert(LidarDefaults::SENSOR_TIMEOUT_MS == 100);

void setup() {}
void loop() {}

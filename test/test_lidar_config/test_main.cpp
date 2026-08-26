#include <Arduino.h>
#include "lidar_config.h"

constexpr LidarConfigArray withFirstConfig(const LidarConfig& first) {
    return {{first,
             SENSOR_CONFIGS[1],
             SENSOR_CONFIGS[2],
             SENSOR_CONFIGS[3],
             SENSOR_CONFIGS[4],
             SENSOR_CONFIGS[5],
             SENSOR_CONFIGS[6],
             SENSOR_CONFIGS[7],
             SENSOR_CONFIGS[8],
             SENSOR_CONFIGS[9],
             SENSOR_CONFIGS[10],
             SENSOR_CONFIGS[11]}};
}

constexpr LidarConfigArray DUPLICATE_PIN_CONFIGS =
    withFirstConfig({13, 0x30, {4, 4, 8, 7}, 800, 850});
constexpr LidarConfigArray DUPLICATE_ADDRESS_CONFIGS =
    withFirstConfig({4, 0x31, {4, 4, 8, 7}, 800, 850});
constexpr LidarConfigArray RESERVED_ADDRESS_CONFIGS =
    withFirstConfig({4, 0x29, {4, 4, 8, 7}, 800, 850});
constexpr LidarConfigArray LOW_ADDRESS_CONFIGS =
    withFirstConfig({4, 0x07, {4, 4, 8, 7}, 800, 850});
constexpr LidarConfigArray HIGH_ADDRESS_CONFIGS =
    withFirstConfig({4, 0x78, {4, 4, 8, 7}, 800, 850});
constexpr LidarConfigArray INVALID_HYSTERESIS_CONFIGS =
    withFirstConfig({4, 0x30, {4, 4, 8, 7}, 800, 800});
constexpr LidarConfigArray EDGE_CROSSING_ROI_CONFIGS =
    withFirstConfig({4, 0x30, {4, 4, 1, 7}, 800, 850});

static_assert(SENSOR_COUNT == 12, "A harpa deve ter 12 sensores");
static_assert(spadNumberFromCoordinates(0, 0) == 128);
static_assert(spadNumberFromCoordinates(15, 15) == 0);
static_assert(spadNumberFromCoordinates(8, 7) == 199);
static_assert(isValidRoi({4, 4, 8, 7}));
static_assert(!isValidRoi({3, 4, 8, 7}));
static_assert(configsAreValid(SENSOR_CONFIGS));
static_assert(!configsAreValid(DUPLICATE_PIN_CONFIGS));
static_assert(!configsAreValid(DUPLICATE_ADDRESS_CONFIGS));
static_assert(!configsAreValid(RESERVED_ADDRESS_CONFIGS));
static_assert(!configsAreValid(LOW_ADDRESS_CONFIGS));
static_assert(!configsAreValid(HIGH_ADDRESS_CONFIGS));
static_assert(!configsAreValid(INVALID_HYSTERESIS_CONFIGS));
static_assert(!configsAreValid(EDGE_CROSSING_ROI_CONFIGS));
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

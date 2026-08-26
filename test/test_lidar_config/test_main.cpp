#include <Arduino.h>
#include "lidar_config.h"

static_assert(SENSOR_COUNT == 12, "A harpa deve ter 12 sensores");
static_assert(spadNumberFromCoordinates(0, 0) == 128);
static_assert(spadNumberFromCoordinates(15, 15) == 0);
static_assert(spadNumberFromCoordinates(8, 7) == 199);
static_assert(isValidRoi({4, 4, 8, 7}));
static_assert(!isValidRoi({3, 4, 8, 7}));
static_assert(configsAreValid(SENSOR_CONFIGS));

void setup() {}
void loop() {}

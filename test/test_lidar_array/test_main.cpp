#include <Arduino.h>
#include <unity.h>

#include "lidar_array.h"

void test_empty_array_starts_unavailable() {
    LidarArray lidars;

    TEST_ASSERT_EQUAL_UINT8(0, lidars.availableCount());
    TEST_ASSERT_EQUAL_UINT32(SENSOR_COUNT, lidars.readings().size());
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_empty_array_starts_unavailable);
    UNITY_END();
}

void loop() {}

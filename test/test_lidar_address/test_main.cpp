#include <Arduino.h>
#include <unity.h>

#include "measurement_status.h"

void test_successful_measurement_is_valid() {
    TEST_ASSERT_EQUAL(MeasurementStatus::Valid,
                      classifyMeasurement(false));
}

void test_timed_out_measurement_is_rejected() {
    TEST_ASSERT_EQUAL(MeasurementStatus::TimedOut,
                      classifyMeasurement(true));
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_successful_measurement_is_valid);
    RUN_TEST(test_timed_out_measurement_is_rejected);
    UNITY_END();
}

void loop() {}

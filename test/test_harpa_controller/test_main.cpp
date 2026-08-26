#include <Arduino.h>
#include <unity.h>

#include "harpa_controller.h"

#include "../../src/harpa_controller.cpp"

HarpaController controller;
LidarReadingArray readings{};

void setUp() {
    controller = HarpaController{};
    readings = {};
}

void test_press_hold_and_release_use_hysteresis() {
    readings[0] = {799, 10, true};
    HarpaFrame frame = controller.update(readings, 10);
    TEST_ASSERT_BITS_HIGH(1u, frame.pressedMask);
    TEST_ASSERT_BITS_HIGH(1u, frame.activeMask);

    readings[0] = {825, 20, true};
    frame = controller.update(readings, 20);
    TEST_ASSERT_EQUAL_UINT16(0, frame.pressedMask);
    TEST_ASSERT_EQUAL_UINT16(0, frame.releasedMask);
    TEST_ASSERT_BITS_HIGH(1u, frame.activeMask);

    readings[0] = {851, 30, true};
    frame = controller.update(readings, 30);
    TEST_ASSERT_BITS_HIGH(1u, frame.releasedMask);
    TEST_ASSERT_BITS_LOW(1u, frame.activeMask);
}

void test_invalid_or_stale_reading_releases_active_string() {
    readings[2] = {700, 10, true};
    controller.update(readings, 10);
    HarpaFrame frame = controller.update(readings, 111);
    TEST_ASSERT_BITS_HIGH(1u << 2, frame.releasedMask);
    TEST_ASSERT_BITS_LOW(1u << 2, frame.activeMask);
}

void test_distance_at_trigger_threshold_does_not_press() {
    readings[0] = {800, 10, true};

    const HarpaFrame frame = controller.update(readings, 10);

    TEST_ASSERT_BITS_LOW(1u, frame.pressedMask);
    TEST_ASSERT_BITS_LOW(1u, frame.activeMask);
}

void test_distance_at_release_threshold_does_not_release() {
    readings[0] = {799, 10, true};
    controller.update(readings, 10);
    readings[0] = {850, 20, true};

    const HarpaFrame frame = controller.update(readings, 20);

    TEST_ASSERT_BITS_LOW(1u, frame.releasedMask);
    TEST_ASSERT_BITS_HIGH(1u, frame.activeMask);
}

void test_two_strings_press_independent_bits() {
    readings[1] = {700, 10, true};
    readings[10] = {700, 10, true};

    const HarpaFrame frame = controller.update(readings, 10);

    const uint16_t expectedMask = (1u << 1) | (1u << 10);
    TEST_ASSERT_EQUAL_UINT16(expectedMask, frame.pressedMask);
    TEST_ASSERT_EQUAL_UINT16(expectedMask, frame.activeMask);
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_press_hold_and_release_use_hysteresis);
    RUN_TEST(test_invalid_or_stale_reading_releases_active_string);
    RUN_TEST(test_distance_at_trigger_threshold_does_not_press);
    RUN_TEST(test_distance_at_release_threshold_does_not_release);
    RUN_TEST(test_two_strings_press_independent_bits);
    UNITY_END();
}

void loop() {}

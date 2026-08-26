#include "lidar_sensor.h"

#include <Arduino.h>

void LidarSensor::holdInReset(const LidarConfig& config) {
    config_ = config;
    pinMode(config_.xshutPin, OUTPUT);
    digitalWrite(config_.xshutPin, LOW);
    reading_ = {};
    configured_ = false;
    available_ = false;
}

bool LidarSensor::begin(TwoWire& bus) {
    pinMode(config_.xshutPin, INPUT);
    delay(10);

    driver_.setBus(&bus);
    driver_.setTimeout(LidarDefaults::SENSOR_TIMEOUT_MS);

    if (!driver_.init()) {
        failAndReset();
        return false;
    }

    driver_.setAddress(config_.i2cAddress);
    if (driver_.last_status != 0) {
        failAndReset();
        return false;
    }

    if (!driver_.setDistanceMode(VL53L1X::Short) ||
        driver_.last_status != 0 ||
        !driver_.setMeasurementTimingBudget(
            LidarDefaults::MEASUREMENT_BUDGET_US) ||
        driver_.last_status != 0) {
        failAndReset();
        return false;
    }

    driver_.setROISize(config_.roi.width, config_.roi.height);
    if (driver_.last_status != 0) {
        failAndReset();
        return false;
    }

    driver_.setROICenter(
        spadNumberFromCoordinates(config_.roi.centerX, config_.roi.centerY));
    if (driver_.last_status != 0) {
        failAndReset();
        return false;
    }

    driver_.startContinuous(LidarDefaults::MEASUREMENT_PERIOD_MS);
    if (driver_.last_status != 0) {
        failAndReset();
        return false;
    }

    configured_ = true;
    available_ = true;
    return true;
}

void LidarSensor::service(uint32_t nowMs) {
    if (!available_ || !driver_.dataReady()) {
        return;
    }

    reading_.distanceMm = driver_.read(false);
    reading_.valid = !driver_.timeoutOccurred() &&
                     driver_.ranging_data.range_status ==
                         VL53L1X::RangeValid;
    reading_.updatedAtMs = nowMs;
}

const LidarReading& LidarSensor::reading() const { return reading_; }

bool LidarSensor::available() const { return available_; }

void LidarSensor::failAndReset() {
    pinMode(config_.xshutPin, OUTPUT);
    digitalWrite(config_.xshutPin, LOW);
    configured_ = false;
    available_ = false;
}

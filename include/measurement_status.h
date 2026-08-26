#pragma once

enum class MeasurementStatus {
    Valid,
    TimedOut,
};

constexpr MeasurementStatus classifyMeasurement(bool timedOut) {
    return timedOut ? MeasurementStatus::TimedOut : MeasurementStatus::Valid;
}

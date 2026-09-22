#pragma once

#include "harpa_array.h"

struct RoiConfig {
    uint8_t width;
    uint8_t height;
    uint8_t centerX;
    uint8_t centerY;
};

struct LidarConfig {
    uint8_t xshutPin;
    uint8_t i2cAddress;
    RoiConfig roi;
    uint16_t triggerMm;
    uint16_t releaseMm;
};

constexpr size_t SENSOR_COUNT = 10;
static_assert(SENSOR_COUNT == 10, "A harpa deve ter exatamente 10 sensores");
using LidarConfigArray = harpa_stl::Array<LidarConfig, SENSOR_COUNT>;

constexpr uint8_t spadNumberFromCoordinates(uint8_t x, uint8_t y) {
    return y < 8 ? static_cast<uint8_t>(128 + x * 8 + y)
                 : static_cast<uint8_t>(127 - x * 8 - (y - 8));
}

constexpr bool isValidRoi(const RoiConfig& roi) {
    if (roi.width < 4 || roi.width > 16 || roi.height < 4 ||
        roi.height > 16 || roi.centerX > 15 || roi.centerY > 15) {
        return false;
    }
    const uint8_t left = roi.width / 2;
    const uint8_t right = roi.width - left - 1;
    const uint8_t top = (roi.height - 1) / 2;
    const uint8_t bottom = roi.height - top - 1;
    return roi.centerX >= left && roi.centerX + right < 16 &&
           roi.centerY >= top && roi.centerY + bottom < 16;
}

constexpr bool isValidRoi(uint8_t width, uint8_t height, uint8_t centerX,
                          uint8_t centerY) {
    return isValidRoi(RoiConfig{width, height, centerX, centerY});
}

constexpr bool addressesAreUnique(const LidarConfigArray& configs,
                                  size_t index, size_t prior) {
    return prior == index
               ? true
               : (configs[index].i2cAddress != configs[prior].i2cAddress &&
                  addressesAreUnique(configs, index, prior + 1));
}

constexpr bool configsAreValidAt(const LidarConfigArray& configs,
                                 size_t index) {
    return index == SENSOR_COUNT
               ? true
               : (configs[index].xshutPin != 0 &&
                  configs[index].i2cAddress >= 0x08 &&
                  configs[index].i2cAddress <= 0x77 &&
                  configs[index].i2cAddress != 0x29 &&
                  isValidRoi(configs[index].roi) &&
                  configs[index].releaseMm > configs[index].triggerMm &&
                  addressesAreUnique(configs, index, 0) &&
                  configsAreValidAt(configs, index + 1));
}

constexpr bool pinsAreUniqueAt(const LidarConfigArray& configs, size_t index,
                               size_t prior) {
    return prior == index
               ? true
               : (configs[index].xshutPin != configs[prior].xshutPin &&
                  pinsAreUniqueAt(configs, index, prior + 1));
}

constexpr bool pinsAreUniqueAllAt(const LidarConfigArray& configs,
                                  size_t index) {
    return index == SENSOR_COUNT
               ? true
               : (pinsAreUniqueAt(configs, index, 0) &&
                  pinsAreUniqueAllAt(configs, index + 1));
}

constexpr bool configsAreValid(const LidarConfigArray& configs) {
    return configsAreValidAt(configs, 0) && pinsAreUniqueAllAt(configs, 0);
}

// Pinos XSHUT mapeados para o Arduino Leonardo (ATmega32u4).
// Indisponiveis: 0/1 (Serial RX/TX) e 2/3 (I2C SDA/SCL).
// Usamos os digitais livres 4-13 (10 sensores).
constexpr LidarConfigArray SENSOR_CONFIGS{{
    {4,  0x30, {4, 4, 8, 7}, 800, 850},
    {5,  0x31, {4, 4, 8, 7}, 800, 850},
    {6,  0x32, {4, 4, 8, 7}, 800, 850},
    {7,  0x33, {4, 4, 8, 7}, 800, 850},
    {8,  0x34, {4, 4, 8, 7}, 800, 850},
    {9,  0x35, {4, 4, 8, 7}, 800, 850},
    {10, 0x36, {4, 4, 8, 7}, 800, 850},
    {11, 0x37, {4, 4, 8, 7}, 800, 850},
    {12, 0x38, {4, 4, 8, 7}, 800, 850},
    {13, 0x39, {4, 4, 8, 7}, 800, 850},
}};

static_assert(configsAreValid(SENSOR_CONFIGS),
              "Configuracao invalida de pino, endereco, ROI ou histerese");

namespace LidarDefaults {
// No Leonardo (ATmega32u4) o barramento I2C usa pinos fixos: SDA = 2, SCL = 3.
// Nao sao configuraveis por software, portanto so definimos o clock.
constexpr uint32_t I2C_CLOCK_HZ = 400000;
constexpr uint32_t MEASUREMENT_BUDGET_US = 20000;
constexpr uint32_t MEASUREMENT_PERIOD_MS = 25;
constexpr uint32_t SENSOR_TIMEOUT_MS = 100;
constexpr uint32_t STALE_AFTER_MS = 100;
constexpr bool ENABLE_DISTANCE_TELEMETRY = false;
}  // namespace LidarDefaults

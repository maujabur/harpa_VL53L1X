#pragma once

#include <stdint.h>

// Coordenadas da matriz SPAD vistas de frente para o encapsulamento:
// X: 0 = esquerda, 15 = direita
// Y: 0 = cima,     15 = baixo
// A lente inverte a imagem; mover a ROI na matriz desloca o campo observado
// na direcao oposta.
namespace SensorConfig {
constexpr uint8_t ROI_WIDTH = 4;
constexpr uint8_t ROI_HEIGHT = 4;
constexpr uint8_t ROI_CENTER_X = 8;
constexpr uint8_t ROI_CENTER_Y = 7;
}  // namespace SensorConfig

constexpr uint8_t spadNumberFromCoordinates(uint8_t x, uint8_t y) {
    return y < 8 ? static_cast<uint8_t>(128 + x * 8 + y)
                 : static_cast<uint8_t>(127 - x * 8 - (y - 8));
}

constexpr bool isValidRoi(uint8_t width, uint8_t height,
                          uint8_t centerX, uint8_t centerY) {
    if (width < 4 || width > 16 || height < 4 || height > 16 ||
        centerX > 15 || centerY > 15) {
        return false;
    }

    // Para centros entre SPADs, a ST especifica o SPAD da direita e/ou acima.
    const uint8_t left = width / 2;
    const uint8_t right = width - left - 1;
    const uint8_t top = (height - 1) / 2;
    const uint8_t bottom = height - top - 1;

    return centerX >= left && centerX + right < 16 &&
           centerY >= top && centerY + bottom < 16;
}

static_assert(
    isValidRoi(SensorConfig::ROI_WIDTH, SensorConfig::ROI_HEIGHT,
               SensorConfig::ROI_CENTER_X, SensorConfig::ROI_CENTER_Y),
    "ROI invalida: use tamanho entre 4 e 16 e mantenha toda a ROI dentro da matriz");

constexpr uint8_t ROI_CENTER_SPAD = spadNumberFromCoordinates(
    SensorConfig::ROI_CENTER_X, SensorConfig::ROI_CENTER_Y);

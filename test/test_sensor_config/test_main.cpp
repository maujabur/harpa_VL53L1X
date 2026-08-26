#include <Arduino.h>

#include "sensor_config.h"

static_assert(spadNumberFromCoordinates(0, 0) == 128,
              "O canto superior esquerdo deve mapear para o SPAD 128");
static_assert(spadNumberFromCoordinates(15, 15) == 0,
              "O canto inferior direito deve mapear para o SPAD 0");
static_assert(spadNumberFromCoordinates(8, 7) == 199,
              "O centro padrao deve mapear para o SPAD 199");

static_assert(isValidRoi(16, 16, 8, 7),
              "A ROI completa centralizada deve ser valida");
static_assert(isValidRoi(4, 4, 2, 1),
              "Uma ROI 4x4 dentro da matriz deve ser valida");
static_assert(!isValidRoi(3, 4, 2, 1),
              "Larguras menores que quatro devem ser rejeitadas");
static_assert(!isValidRoi(4, 4, 15, 1),
              "Uma ROI que ultrapassa a lateral deve ser rejeitada");
static_assert(!isValidRoi(4, 4, 2, 14),
              "Uma ROI que ultrapassa a parte inferior deve ser rejeitada");

void setup() {}

void loop() {}

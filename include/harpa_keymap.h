#pragma once

#include <stddef.h>

#include "lidar_config.h"

// Mapeamento entre cada sensor da harpa (indice 0..SENSOR_COUNT-1) e a tecla
// que ele deve emular via USB HID. A ordem segue a ordem dos sensores em
// SENSOR_CONFIGS. Edite a string abaixo para trocar o layout do teclado.
//
// Cada caractere e enviado a biblioteca Keyboard do Arduino: para caracteres
// ASCII imprimiveis (letras, digitos, pontuacao) o proprio valor do char ja e
// o codigo aceito por Keyboard.press()/release().
namespace HarpaKeymap {

// 10 teclas para 10 sensores.
constexpr char KEYS[] = "zxcvbnm,./";

// Numero de teclas definidas (exclui o terminador nulo da string literal).
constexpr size_t KEY_COUNT = sizeof(KEYS) - 1;

static_assert(KEY_COUNT == SENSOR_COUNT,
              "A quantidade de teclas em KEYS deve ser igual a SENSOR_COUNT");

// Retorna a tecla associada ao sensor de indice informado.
constexpr char keyForSensor(size_t index) { return KEYS[index]; }

}  // namespace HarpaKeymap

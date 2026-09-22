#pragma once

// O core AVR do Arduino (avr-gcc) nao fornece a STL, entao nao existe
// <array>, <cstdint> nem <cstddef>. Este header oferece:
//   - inclusao dos tipos inteiros e size_t via headers C (<stdint.h>/<stddef.h>)
//   - uma implementacao minima e constexpr de um array de tamanho fixo,
//     compativel com o subconjunto de std::array usado neste projeto
//     (operator[], size(), inicializacao por lista e atribuicao/copia).
//
// A API fica em harpa_stl::Array para nao colidir com std::array quando
// o codigo for compilado no host (testes nativos), onde a STL existe.

#include <stddef.h>
#include <stdint.h>

namespace harpa_stl {

template <typename T, size_t N>
struct Array {
    T data_[N];

    constexpr size_t size() const { return N; }

    T& operator[](size_t index) { return data_[index]; }
    constexpr const T& operator[](size_t index) const { return data_[index]; }

    T* begin() { return data_; }
    T* end() { return data_ + N; }
    constexpr const T* begin() const { return data_; }
    constexpr const T* end() const { return data_ + N; }
};

}  // namespace harpa_stl

#pragma once
#include <cstdint>

namespace DESHelper
{
    constexpr uint32_t ENCRYPT = 1;
    constexpr uint32_t DECRYPT = 0;

    void TripleDESKeySetup(const uint8_t* key, uint8_t schedule[3][16][6], uint32_t mode);
    void TripleDESCrypt(const uint8_t* input, uint8_t* output, uint8_t key[3][16][6]);
    void qmc1_decrypt(uint8_t* data, size_t size);

}

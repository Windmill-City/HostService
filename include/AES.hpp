#pragma once
#include <array>

struct AES
{
    std::array<uint8_t, 12>      Nonce;
    std::array<uint8_t, 256 / 8> Key;
};

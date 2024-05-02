#pragma once
#include <array>

#define NONCE_SIZE_IN_BYTE 12
#define AES_KEY_SIZE_IN_BIT 256

struct AES
{
    std::array<uint8_t, NONCE_SIZE_IN_BYTE>      Nonce;
    std::array<uint8_t, AES_KEY_SIZE_IN_BIT / 8> Key;
};

#pragma once

#include <cstdint>

constexpr auto bits_per_sample = 16;
constexpr auto adpcm_block_size = 36;
constexpr auto pcm_block_size = 128;

void adpcm_decode_block(uint8_t* src, int16_t* dest);
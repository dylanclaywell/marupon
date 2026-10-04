#pragma once

#include <Arduino.h>

constexpr uint16_t panelColor(uint8_t r, uint8_t g, uint8_t b)
{
  uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  // This board (Waveshare ESP32-S3 display) requires the RGB565 color to be byte-swapped
  return __builtin_bswap16(rgb565);
}
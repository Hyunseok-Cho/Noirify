#pragma once
#include <cstdint>

extern "C" {

    void set_rgb(uint8_t r, uint8_t g, uint8_t b);

    void to_grayscale(uint8_t* buffer, int width, int height);

}
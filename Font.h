#pragma once

#include <cstdint>

struct Font
{
    const uint16_t* const data;
    const uint8_t width;
    const uint8_t height;
};

extern const Font Font6x8;
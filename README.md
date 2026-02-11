# SSD1306 OLED Display Driver

A C++ driver for SSD1306-based 128x64 pixel I2C OLED displays, designed for STM32F4 microcontrollers using the HAL library.

## Features

- DMA support with double buffering for asynchronous screen updates
- Dirty-page tracking for efficient partial refreshes for synchronous screen update
- Graphics primitives (pixels, rectangles, circles, images)
- Text rendering with bitmap fonts

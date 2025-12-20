#include "Display.h"

void Display::Init(I2C_HandleTypeDef* hi2c)
{
    _hi2c = hi2c;

    WriteCommand(Command::SetDisplayOff);

    WriteCommand(Command::SetMultiplexRatio);
    SetDisplayOffset(0);
    SetComPinsHardwareConf(true, false);
    SetContrast(0xFF);
    SetDivideRatioAndOSC();
    WriteCommand(Command::EnableChargePump);
    SetPreChargePeriod();
    SetVComhDeselectLevel();

    WriteCommand(Command::PageMode);
    SetPageStartAddress(0);
    SetLowColumnStartAddress(0);
    SetHighColumnStartAddress(0);
    SetDisplayStartLine(0);
    ResetColumnAddress();
    ResetPageAddress();

    WriteCommand(Command::SetSegmentRemap127To0);
    WriteCommand(Command::SetComOutputScanDirection);

    WriteCommand(Command::NormalDisplay);
    WriteCommand(Command::ResumeToRAMContent);
    WriteCommand(Command::SetDisplayOn);

    FillBlack();
    UpdateScreen();
}

void Display::WriteCommand(uint8_t* command, size_t size)
{
    HAL_I2C_Mem_Write(_hi2c, SSD1306_I2C_ADDRESS, static_cast<uint8_t>(ControlByte::Command), 1, command, size, HAL_MAX_DELAY);
}

void Display::WriteCommand(uint8_t command)
{
    WriteCommand(&command, 1);
}

void Display::WriteData(uint8_t* data, size_t size)
{
    HAL_I2C_Mem_Write(_hi2c, SSD1306_I2C_ADDRESS, static_cast<uint8_t>(ControlByte::Data), 1, data, size, HAL_MAX_DELAY);
}

void Display::UpdateScreen()
{
    for (int i = 0; i < PAGES_COUNT; ++i)
    {
        if (_dirtyFlags[i])
        {
            SetPageStartAddress(i);
            SetLowColumnStartAddress(0);
            SetHighColumnStartAddress(0);
            WriteData(_buffer.data() + i * DISPLAY_WIDTH, DISPLAY_WIDTH);
            _dirtyFlags[i] = false;
        }
    }
}

void Display::ResetColumnAddress()
{
    const uint8_t command[3] = {Command::SetColumnAddress, 0, DISPLAY_WIDTH - 1};
    WriteCommand(command);
}

void Display::ResetPageAddress()
{
    const uint8_t command[3] = {Command::SetPageAddress, 0, PAGES_COUNT - 1};
    WriteCommand(command);
}

void Display::SetPageStartAddress(uint8_t page)
{
    const uint8_t command = Command::SetPageStartAddress | page;
    WriteCommand(command);
}

void Display::SetContrast(uint8_t contrast)
{
    const uint8_t command[2] = {Command::SetContrastControl, contrast};
    WriteCommand(command);
}

void Display::SetDisplayOffset(uint8_t offset)
{
    const uint8_t command[2] = {Command::SetDisplayOffset, offset};
    WriteCommand(command);
}

void Display::SetLowColumnStartAddress(uint8_t address)
{
    const uint8_t command = 0x0F & address;
    WriteCommand(command);
}

void Display::SetHighColumnStartAddress(uint8_t address)
{
    const uint8_t command = 0x1F & address;
    WriteCommand(command);
}

void Display::SetDisplayStartLine(uint8_t line)
{
    line &= 0x1F; // 0b00111111
    const uint8_t command = 0x40 | line; // 0b01000000 | line
    WriteCommand(command);
}

void Display::SetDivideRatioAndOSC(uint8_t divideRatio, uint8_t oscillatorFrequency)
{
    const uint8_t command[2] = {Command::SetDivideRatioAndOSC, (oscillatorFrequency << 4) | divideRatio};
    WriteCommand(command);
}

void Display::SetPreChargePeriod(uint8_t phase1, uint8_t phase2)
{
    const uint8_t command[2] = {Command::SetPreChargePeriod, (phase2 << 4) | phase1};
    WriteCommand(command);
}

void Display::SetComPinsHardwareConf(bool alternativeConf, bool leftRightRemapping)
{
    uint8_t command[2] = {Command::SetComPinsHardwareConf, 0x02};
    if (alternativeConf)
    {
        command[1] |= 1 << 4;
    }
    if (leftRightRemapping)
    {
        command[1] |= 1 << 5;
    }
    WriteCommand(command);
}

void Display::SetVComhDeselectLevel(VComhDeselectLevel level)
{
    const uint8_t command[2] = {Command::SetVComhDeselectLevel, static_cast<uint8_t>(level)};
    WriteCommand(command);
}

void Display::FillBlack()
{
    _buffer.fill(0x00);
    _dirtyFlags.fill(true);
}

void Display::DrawImage(const std::array<uint8_t, PAGES_SIZE>& image)
{
    _buffer = image;
    _dirtyFlags.fill(true);
}

void Display::DrawPixel(uint8_t x, uint8_t y, bool color)
{
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
    {
        return;
    }

    const uint8_t page = y / 8;
    const size_t index = page * DISPLAY_WIDTH + x;
    const uint8_t bitPosition = y % 8;

    if (color)
    {
        _buffer[index] |= (1 << bitPosition);
    }
    else
    {
        _buffer[index] &= ~(1 << bitPosition);
    }

    _dirtyFlags[page] = true;
}

void Display::DrawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool color)
{
    for (uint8_t i = x; i < x + width; ++i)
    {
        for (uint8_t j = y; j < y + height; ++j)
        {
            DrawPixel(i, j, color);
        }
    }
}

void Display::DrawCircle(uint8_t x0, uint8_t y0, uint8_t r, bool color, bool fill)
{
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    fill ? FillOctant(x0, y0, x, y, color) : DrawOctant(x0, y0, x, y, color);

    while (y >= x)
    {
        if (d > 0)
        {
            y--;
            d += 4 * (x - y) + 10;
        }
        else
        {
            d += 4 * x + 6;
        }

        x++;

        fill ? FillOctant(x0, y0, x, y, color) : DrawOctant(x0, y0, x, y, color);
    }
}

void Display::DrawOctant(uint8_t x0, uint8_t y0, uint8_t x, uint8_t y, bool color)
{
    DrawPixel(x0 + x, y0 + y, color);
    DrawPixel(x0 - x, y0 + y, color);
    DrawPixel(x0 + x, y0 - y, color);
    DrawPixel(x0 - x, y0 - y, color);
    DrawPixel(x0 + y, y0 + x, color);
    DrawPixel(x0 - y, y0 + x, color);
    DrawPixel(x0 + y, y0 - x, color);
    DrawPixel(x0 - y, y0 - x, color);
}

void Display::FillOctant(uint8_t x0, uint8_t y0, uint8_t x, uint8_t y, bool color)
{
    for (int i = x0 - x; i <= x0 + x; ++i)
    {
        DrawPixel(i, y0 + y, color);
        DrawPixel(i, y0 - y, color);
    }
    for (int i = x0 - y; i <= x0 + y; ++i)
    {
        DrawPixel(i, y0 + x, color);
        DrawPixel(i, y0 - x, color);
    }
}

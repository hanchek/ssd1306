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

    WriteCommand(Command::HorizontalMode);
    WriteCommand(Command::SetPageStartAddress);
    SetLowColumnStartAddress(0);
    SetHighColumnStartAddress(0);
    SetDisplayStartLine(0);

    WriteCommand(Command::SetSegmentRemap127To0);
    WriteCommand(Command::SetComOutputScanDirectionReversed);

    WriteCommand(Command::NormalDisplay);
    WriteCommand(Command::ResumeToRAMContent);
    WriteCommand(Command::SetDisplayOn);
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

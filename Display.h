#include <cstdint>
#include <cstring>

#include <stm32f4xx_hal.h>

// docs: https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf

constexpr uint16_t SSD1306_I2C_ADDRESS = 0x3C << 1; // 0b111100
constexpr uint8_t DISPLAY_HEIGHT = 64;

// Control byte consists of Co and D/C# bits following by six "0" bits
enum class ControlByte : uint8_t
{
    Command = 0x00, // 0b00000000
    Data = 0x40, // 0b01000000
};

namespace Command
{
    constexpr uint8_t ResumeToRAMContent = 0xA4;
    constexpr uint8_t EntireDisplayOn = 0xA5;
    constexpr uint8_t NormalDisplay = 0xA6;
    constexpr uint8_t InverseDisplay = 0xA7;
    constexpr uint8_t SetDisplayOff = 0xAE;
    constexpr uint8_t SetDisplayOn = 0xAF;

    // next two commands allow to mirror horizontally
    constexpr uint8_t SetSegmentRemap0To127 = 0xA0;
    constexpr uint8_t SetSegmentRemap127To0 = 0xA1;

    // next two commands allow to mirror vertically
    constexpr uint8_t SetComOutputScanDirection = 0xC0; // normal mode (RESET) Scan from COM0 to COM[N –1] 
    constexpr uint8_t SetComOutputScanDirectionReversed = 0xC8; // remapped mode. Scan from COM[N-1] to COM0

    constexpr uint8_t HorizontalMode[2] = {0x20, 0x00};
    constexpr uint8_t VerticalMode[2] = {0x20, 0x01};
    constexpr uint8_t PageMode[2] = {0x20, 0x02};

    constexpr uint8_t SetPageStartAddress = 0xB0; // 3 bits for page start address, so 0xB0 to 0xB7. page 0 to 7

    constexpr uint8_t SetContrastControl = 0x81; // second byte is contrast value
    constexpr uint8_t SetMultiplexRatio[2] = {0xA8, DISPLAY_HEIGHT - 1};
    constexpr uint8_t SetDisplayOffset = 0xD3; // second byte is display offset
    constexpr uint8_t SetDivideRatioAndOSC = 0xD5; // second byte: first 4 bits are divide ratio, last 4 bits are oscillator frequency
    constexpr uint8_t SetPreChargePeriod = 0xD9; // second byte: first 4 bits are phase1, last 4 bits are phase2
    constexpr uint8_t SetComPinsHardwareConf = 0xDA; // second byte: 00xy0010, x=0 for sequential, x=1 for alternative, y=0 disable left/right remapping, y=1 enable left/right remapping
    constexpr uint8_t SetVComhDeselectLevel = 0xDB; // second byte is vcomh deselect level

    constexpr uint8_t DisableChargePump[2] = {0x8D, 0x10};
    constexpr uint8_t EnableChargePump[2] = {0x8D, 0x14};

};

enum class VComhDeselectLevel : uint8_t
{
    VComh0_65Vcc = 0x00,
    VComh0_77Vcc = 0x20, // 0b00100000
    VComh0_83Vcc = 0x30 // 0b00110000
};

class Display
{
    public:
        void Init(I2C_HandleTypeDef* hi2c);

        void WriteCommand(uint8_t* command, size_t size);
        void WriteCommand(uint8_t command);

        template<size_t size>
        void WriteCommand(const uint8_t(&command)[size])
        {
            uint8_t commandBuff[size];
            memcpy(commandBuff, command, size);
            WriteCommand(commandBuff, size);
        }

        void WriteData(uint8_t* data, size_t size);

        void SetContrast(uint8_t contrast);
        void SetDisplayOffset(uint8_t offset);

        /**
        * @brief Set the low column start address.
        * @param address The low column start address. from 0 to 15.
        */
        void SetLowColumnStartAddress(uint8_t address);

        /**
        * @brief Set the high column start address.
        * @param address The high column start address. from 0 to 15.
        */
        void SetHighColumnStartAddress(uint8_t address);

        /**
        * @brief Set the display start line.
        * @param line The display start line. from 0 to 63.
        */
        void SetDisplayStartLine(uint8_t line);

        /**
        * @brief Set the divide ratio and oscillator frequency.
        * @param divideRatio The divide ratio. 1 to 15. RESET is 0.
        * @param oscillatorFrequency The oscillator frequency. 0 to 15
        */
        void SetDivideRatioAndOSC(uint8_t divideRatio = 0, uint8_t oscillatorFrequency = 15);

        /**
        * @brief Set the pre-charge period.
        * @param phase1 The first phase of the pre-charge period. 1 to 15. RESET is 0x2
        * @param phase2 The second phase of the pre-charge period. 1 to 15. RESET is 0x2
        */
        void SetPreChargePeriod(uint8_t phase1 = 0x02, uint8_t phase2 = 0x02);

        /**
        * @brief Set the com pins hardware configuration.
        * @param alternativeConf If true, the com pins are configured in alternative mode, otherwise sequential.
        * @param leftRightRemapping If true, the left/right remapping is enabled.
        */
        void SetComPinsHardwareConf(bool alternativeConf = false, bool leftRightRemapping = false);

        /**
        * @brief Set the vcomh deselect level.
        * @param level The vcomh deselect level.
        * @note 0.65 is more contrast but unstable, possible ghosting.
        * @note 0.77 is default.
        * @note 0.83 is lower contrast, lower power.
        */
        void SetVComhDeselectLevel(VComhDeselectLevel level = VComhDeselectLevel::VComh0_77Vcc);

    protected:
        I2C_HandleTypeDef* _hi2c;
};

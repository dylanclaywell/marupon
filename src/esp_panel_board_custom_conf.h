#pragma once

// Custom board: Waveshare ESP32-S3-Touch-LCD-1.46. Pins from
// https://docs.waveshare.com/ESP32-S3-Touch-LCD-1.46
#define ESP_PANEL_BOARD_DEFAULT_USE_CUSTOM (1)

#if ESP_PANEL_BOARD_DEFAULT_USE_CUSTOM
#define ESP_PANEL_BOARD_NAME "Waveshare:ESP32_S3_TOUCH_LCD_1_46"
#define ESP_PANEL_BOARD_WIDTH (412)
#define ESP_PANEL_BOARD_HEIGHT (412)

// Turned on one piece at a time. Touch comes next.
#define ESP_PANEL_BOARD_USE_LCD (1)
#define ESP_PANEL_BOARD_USE_TOUCH (0)
#define ESP_PANEL_BOARD_USE_BACKLIGHT (1)

// SPD2010 LCD on a QSPI bus.
#define ESP_PANEL_BOARD_LCD_CONTROLLER SPD2010
#define ESP_PANEL_BOARD_LCD_BUS_TYPE (ESP_PANEL_BUS_TYPE_QSPI)
#define ESP_PANEL_BOARD_LCD_BUS_SKIP_INIT_HOST (0)
#define ESP_PANEL_BOARD_LCD_QSPI_HOST_ID (1)
#define ESP_PANEL_BOARD_LCD_QSPI_IO_SCK (40)
#define ESP_PANEL_BOARD_LCD_QSPI_IO_DATA0 (46)
#define ESP_PANEL_BOARD_LCD_QSPI_IO_DATA1 (45)
#define ESP_PANEL_BOARD_LCD_QSPI_IO_DATA2 (42)
#define ESP_PANEL_BOARD_LCD_QSPI_IO_DATA3 (41)
#define ESP_PANEL_BOARD_LCD_QSPI_IO_CS (21)
#define ESP_PANEL_BOARD_LCD_QSPI_MODE (0)
#define ESP_PANEL_BOARD_LCD_QSPI_CLK_HZ (40 * 1000 * 1000)
#define ESP_PANEL_BOARD_LCD_QSPI_CMD_BITS (32)
#define ESP_PANEL_BOARD_LCD_QSPI_PARAM_BITS (8)
#define ESP_PANEL_BOARD_LCD_COLOR_BITS (ESP_PANEL_LCD_COLOR_BITS_RGB565)
#define ESP_PANEL_BOARD_LCD_COLOR_BGR_ORDER (0)
#define ESP_PANEL_BOARD_LCD_COLOR_INEVRT_BIT (0)
#define ESP_PANEL_BOARD_LCD_SWAP_XY (0)
#define ESP_PANEL_BOARD_LCD_MIRROR_X (0)
#define ESP_PANEL_BOARD_LCD_MIRROR_Y (0)
#define ESP_PANEL_BOARD_LCD_GAP_X (0)
#define ESP_PANEL_BOARD_LCD_GAP_Y (0)
// The LCD reset line is on the IO expander, so it is toggled in the pre-begin hook below.
#define ESP_PANEL_BOARD_LCD_RST_IO (-1)
#define ESP_PANEL_BOARD_LCD_RST_LEVEL (0)

// Backlight brightness by PWM on GPIO 5.
#define ESP_PANEL_BOARD_BACKLIGHT_TYPE (ESP_PANEL_BACKLIGHT_TYPE_PWM_LEDC)
#define ESP_PANEL_BOARD_BACKLIGHT_IO (5)
#define ESP_PANEL_BOARD_BACKLIGHT_ON_LEVEL (1)
#define ESP_PANEL_BOARD_BACKLIGHT_PWM_FREQ_HZ (20000)
#define ESP_PANEL_BOARD_BACKLIGHT_PWM_DUTY_RESOLUTION (10)
#define ESP_PANEL_BOARD_BACKLIGHT_IDLE_OFF (0)

// TCA9554 IO expander on I2C. It drives the LCD and touch reset lines.
#define ESP_PANEL_BOARD_USE_EXPANDER (1)
#define ESP_PANEL_BOARD_EXPANDER_CHIP TCA95XX_8BIT
#define ESP_PANEL_BOARD_EXPANDER_SKIP_INIT_HOST (0)
#define ESP_PANEL_BOARD_EXPANDER_I2C_HOST_ID (1)
#define ESP_PANEL_BOARD_EXPANDER_I2C_CLK_HZ (400 * 1000)
#define ESP_PANEL_BOARD_EXPANDER_I2C_SCL_PULLUP (1)
#define ESP_PANEL_BOARD_EXPANDER_I2C_SDA_PULLUP (1)
#define ESP_PANEL_BOARD_EXPANDER_I2C_IO_SCL (10)
#define ESP_PANEL_BOARD_EXPANDER_I2C_IO_SDA (11)
#define ESP_PANEL_BOARD_EXPANDER_I2C_ADDRESS (0x20)

// Resets the LCD through the expander before the LCD starts (50 ms low, then 50 ms high).
#define ESP_PANEL_BOARD_LCD_PRE_BEGIN_FUNCTION(p)           \
    {                                                       \
        constexpr int LCD_RST = 1;                          \
        auto board = static_cast<Board *>(p);               \
        auto expander = board->getIO_Expander()->getBase(); \
        expander->pinMode(LCD_RST, OUTPUT);                 \
        expander->digitalWrite(LCD_RST, LOW);               \
        vTaskDelay(pdMS_TO_TICKS(50));                      \
        expander->digitalWrite(LCD_RST, HIGH);              \
        vTaskDelay(pdMS_TO_TICKS(50));                      \
        return true;                                        \
    }

// Compatibility check against the library; keep these values.
#define ESP_PANEL_BOARD_CUSTOM_FILE_VERSION_MAJOR 1
#define ESP_PANEL_BOARD_CUSTOM_FILE_VERSION_MINOR 2
#define ESP_PANEL_BOARD_CUSTOM_FILE_VERSION_PATCH 0
#endif
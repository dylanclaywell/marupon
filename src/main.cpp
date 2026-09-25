#include <Arduino.h>
#include <esp_display_panel.hpp>

using namespace esp_panel::board;

static constexpr int SQ = 100;
static uint16_t square[SQ * SQ]; // 20,000 bytes in internal RAM

static inline uint16_t panelColor(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    return __builtin_bswap16(rgb565);
}

// Fills the buffer with one colour and draws it. The -1 waits for the transfer to finish,
// so it is safe to reuse the buffer on the next call.
static void drawSquare(esp_panel::drivers::LCD *lcd, int x, int y, uint16_t color)
{
    for (int i = 0; i < SQ * SQ; i++)
    {
        square[i] = color;
    }
    lcd->drawBitmap(x, y, SQ, SQ, reinterpret_cast<const uint8_t *>(square), -1);
}

static constexpr int FB_WIDTH = 412;
static constexpr int FB_HEIGHT = 412;
static uint16_t *framebuffer = nullptr; // Points into PSRAM once framebufferInit() has run.

// Allocates the framebuffer in PSRAM. Called once at startup and never freed.
// Returns false if there was not enough PSRAM.
static bool framebufferInit()
{
    size_t bytes = FB_WIDTH * FB_HEIGHT * sizeof(uint16_t);
    framebuffer = static_cast<uint16_t *>(ps_malloc(bytes));
    if (framebuffer == nullptr)
    {
        return false;
    }
    return true;
}

void setup()
{
    Serial.begin(115200);
    delay(2000); // Gives the serial monitor time to reconnect after the USB port resets.

    Board *board = new Board();
    board->init();
    assert(board->begin());

    esp_panel::drivers::LCD *lcd = board->getLCD();

    if (framebufferInit())
    {
        drawSquare(lcd, 156, 156, panelColor(0, 255, 255)); // cyan: the framebuffer was allocated
    }
    else
    {
        drawSquare(lcd, 156, 156, panelColor(255, 0, 255)); // magenta: allocation failed
        while (true)
        {
            delay(1000);
        }
    }
}

void loop()
{
    delay(1000);

    Serial.print("Framebuffer at: ");
    Serial.println(reinterpret_cast<uintptr_t>(framebuffer), HEX);
    Serial.print("PSRAM free after: ");
    Serial.println(ESP.getFreePsram());
}
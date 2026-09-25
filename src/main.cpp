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

void setup()
{
    Serial.begin(115200);
    Board *board = new Board();
    board->init();
    assert(board->begin());

    auto lcd = board->getLCD();
    drawSquare(lcd, 56, 104, panelColor(255, 0, 0));      // red
    drawSquare(lcd, 156, 104, panelColor(0, 255, 0));     // green
    drawSquare(lcd, 256, 104, panelColor(0, 0, 255));     // blue
    drawSquare(lcd, 56, 204, panelColor(255, 128, 0));    // orange
    drawSquare(lcd, 156, 204, panelColor(128, 128, 128)); // grey
    drawSquare(lcd, 256, 204, panelColor(255, 255, 255)); // white
}

void loop()
{
    delay(1000);
}
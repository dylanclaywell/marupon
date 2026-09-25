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
static esp_panel::drivers::LCD *lcd = nullptr;

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

// Sets one pixel in the framebuffer. Pixels outside the buffer are ignored.
static inline void setPixel(int x, int y, uint16_t color)
{
    if (x < 0 || x >= FB_WIDTH || y < 0 || y >= FB_HEIGHT)
    {
        return;
    }
    framebuffer[y * FB_WIDTH + x] = color;
}

// Fills a rectangle in the framebuffer. Parts outside the buffer are skipped.
static void fillRect(int x, int y, int w, int h, uint16_t color)
{
    for (int row = y; row < y + h; row++)
    {
        for (int col = x; col < x + w; col++)
        {
            setPixel(col, row, color);
        }
    }
}

// Draws a test image: a dark blue background, a white square outline, and a coloured marker in each
// corner of the outline so a mirrored or rotated image is easy to spot.
static void drawTestPattern()
{
    const uint16_t background = panelColor(0, 0, 64);
    const uint16_t white = panelColor(255, 255, 255);

    fillRect(0, 0, FB_WIDTH, FB_HEIGHT, background);

    // The outline is inset 80 pixels so its corners stay inside the round screen.
    const int inset = 80;
    const int size = FB_WIDTH - 2 * inset;
    fillRect(inset, inset, size, 2, white);            // top edge
    fillRect(inset, inset + size - 2, size, 2, white); // bottom edge
    fillRect(inset, inset, 2, size, white);            // left edge
    fillRect(inset + size - 2, inset, 2, size, white); // right edge

    fillRect(inset + 10, inset + 10, 30, 30, panelColor(255, 0, 0));                 // top left: red
    fillRect(inset + size - 40, inset + 10, 30, 30, panelColor(0, 255, 0));          // top right: green
    fillRect(inset + 10, inset + size - 40, 30, 30, panelColor(0, 0, 255));          // bottom left: blue
    fillRect(inset + size - 40, inset + size - 40, 30, 30, panelColor(255, 255, 0)); // bottom right: yellow
}

static constexpr int STRIP_ROWS = 40; // An arbitrary strip height; we can measure other sizes.

// Sends the whole framebuffer to the panel in horizontal strips. Returns false if any strip failed.
static bool flushFramebuffer(esp_panel::drivers::LCD *lcd)
{
    for (int y = 0; y < FB_HEIGHT; y += STRIP_ROWS)
    {
        int rows = FB_HEIGHT - y;
        if (rows > STRIP_ROWS)
        {
            rows = STRIP_ROWS;
        }
        const uint8_t *data = reinterpret_cast<const uint8_t *>(&framebuffer[y * FB_WIDTH]);
        if (!lcd->drawBitmap(0, y, FB_WIDTH, rows, data, -1))
        {
            return false;
        }
    }
    return true;
}

void setup()
{
    Serial.begin(115200);
    delay(5000); // Gives the serial monitor time to reconnect after the USB port resets.

    Board *board = new Board();
    board->init();
    assert(board->begin());

    lcd = board->getLCD();

    if (framebufferInit())
    {
        drawSquare(lcd, 156, 156, panelColor(0, 255, 255)); // cyan: the framebuffer was allocated

        drawTestPattern();
        flushFramebuffer(lcd);
        Serial.print("Pixel (0,0): 0x");
        Serial.println(framebuffer[0], HEX);
        Serial.print("Pixel (95,95): 0x");
        Serial.println(framebuffer[95 * FB_WIDTH + 95], HEX);
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
    unsigned long start = micros();
    bool ok = flushFramebuffer(lcd);
    unsigned long elapsed = micros() - start;

    if (ok)
    {
        Serial.print("Frame sent in ");
    }
    else
    {
        Serial.print("Frame FAILED after ");
    }
    Serial.print(elapsed);
    Serial.println(" us");

    delay(1000);
}
#include <Arduino.h>
#include <esp_display_panel.hpp>

#include <sprites/sprite_baby.h>

using namespace esp_panel::board;

static constexpr uint16_t panelColor(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    return __builtin_bswap16(rgb565);
}

static constexpr uint16_t TRANSPARENT_COLOR = panelColor(255, 0, 255);

static constexpr int FB_WIDTH = 412;
static constexpr int FB_HEIGHT = 412;
static uint16_t *framebuffer = nullptr; // Points into PSRAM once framebufferInit() has run.
static esp_panel::drivers::LCD *lcd = nullptr;

enum ButtonState
{
    BUTTON_RELEASED,
    BUTTON_PRESSED
};

enum ButtonName
{
    BUTTON_A,
    BUTTON_B,
    BUTTON_UP,
    BUTTON_DOWN,

    BUTTON_COUNT
};

struct ButtonMapping
{
    const char *name;  // the text that arrives over serial, e.g. "UP"
    ButtonName button; // the enum value it stands for, e.g. BUTTON_UP
};

static constexpr ButtonMapping buttonMappings[] = {
    {"A", BUTTON_A},
    {"B", BUTTON_B},
    {"UP", BUTTON_UP},
    {"DOWN", BUTTON_DOWN}};
static constexpr int BUTTON_MAPPING_COUNT = sizeof(buttonMappings) / sizeof(buttonMappings[0]);

static ButtonState buttonStates[BUTTON_COUNT] = {};

static constexpr int LINE_BUFFER_SIZE = 32;
static char lineBuffer[LINE_BUFFER_SIZE];
static int lineLength = 0;

static int spriteX = FB_WIDTH / 2 - SPRITE_BABY_WIDTH / 2;
static int spriteY = FB_HEIGHT / 2 - SPRITE_BABY_HEIGHT / 2;

// Allocates the framebuffer in PSRAM. Called once at startup and never freed.
// Returns false if there was not enough PSRAM.
static bool framebufferInit()
{
    size_t bytes = FB_WIDTH * FB_HEIGHT * sizeof(uint16_t);
    framebuffer = static_cast<uint16_t *>(ps_malloc(bytes));
    return framebuffer != nullptr;
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

static void drawSprite(int x, int y, int w, int h, const uint16_t *data)
{
    for (int row = 0; row < h; row++)
    {
        for (int col = 0; col < w; col++)
        {
            uint16_t color = data[row * w + col];

            // Skip magenta pixels
            if (color == TRANSPARENT_COLOR)
                continue;

            setPixel(x + col, y + row, color);
        }
    }
}

static constexpr int STRIP_ROWS = FB_HEIGHT / 10; // An arbitrary strip height; we can measure other sizes.

// Sends the whole framebuffer to the panel in horizontal strips. Returns false if any strip failed.
static bool flushFramebuffer()
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

static bool findButton(const char *name, ButtonName &out)
{
    for (int i = 0; i < BUTTON_MAPPING_COUNT; i++)
    {
        if (strcmp(name, buttonMappings[i].name) == 0)
        {
            out = buttonMappings[i].button;
            return true;
        }
    }
    return false;
}

void setup()
{
    Serial.begin(115200);
    delay(5000); // Gives the serial monitor time to reconnect after the USB port resets.

    Board *board = new Board();
    board->init();
    bool ok = board->begin();
    assert(ok);

    lcd = board->getLCD();

    if (framebufferInit())
    {
        const uint16_t background = panelColor(0, 0, 64);
        fillRect(0, 0, FB_WIDTH, FB_HEIGHT, background);

        flushFramebuffer();
    }
    else
    {
        Serial.println("Failed to initialize framebuffer.");
        while (true)
        {
            delay(1000);
        }
    }
}

void loop()
{
    flushFramebuffer();

    while (Serial.available() > 0)
    {
        char incomingByte = Serial.read();

        if (incomingByte == '\n')
        {
            lineBuffer[lineLength] = '\0'; // Turn the buffer into a proper C string.
            char buttonName[16];
            char buttonState[16];

            int fieldsParsed = sscanf(lineBuffer, "BUTTON %15s %15s", buttonName, buttonState);

            if (fieldsParsed == 2)
            {
                ButtonName button;
                if (findButton(buttonName, button))
                {
                    buttonStates[button] = (strcmp(buttonState, "PRESSED") == 0) ? BUTTON_PRESSED : BUTTON_RELEASED;
                }
                else
                {
                    Serial.print("Unknown button: ");
                    Serial.println(buttonName);
                }
            }
            else
            {
                Serial.print("Could not parse line: ");
                Serial.println(lineBuffer);
            }

            lineLength = 0; // Start the next line from scratch.
        }
        else if (lineLength < LINE_BUFFER_SIZE - 1) // Leave room for the '\0' above.
        {
            lineBuffer[lineLength] = incomingByte;
            lineLength++;
        }
        // else: buffer is full and this isn't a newline yet -- drop the byte.
    }

    if (buttonStates[BUTTON_UP] == BUTTON_PRESSED)
    {
        spriteY--;
    }
    if (buttonStates[BUTTON_DOWN] == BUTTON_PRESSED)
    {
        spriteY++;
    }

    drawSprite(spriteX, spriteY, SPRITE_BABY_WIDTH, SPRITE_BABY_HEIGHT, sprite_baby_data);
}
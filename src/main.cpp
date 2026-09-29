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

static constexpr uint16_t background = panelColor(0, 0, 64);

static int spriteX = FB_WIDTH / 2 - SPRITE_BABY_WIDTH / 2;
static int spriteY = FB_HEIGHT / 2 - SPRITE_BABY_HEIGHT / 2;

struct Rect
{
    int x;
    int y;
    int w;
    int h;
};

class NullableRect
{
private:
    Rect _rect = Rect{0, 0, 0, 0};

public:
    NullableRect() = default;

    NullableRect(const Rect &rect) : _rect(rect) {}

    void clear()
    {
        _rect = Rect{0, 0, 0, 0};
    }

    bool isEmpty() const
    {
        return _rect.w <= 0 || _rect.h <= 0;
    }

    Rect getRect() const
    {
        return _rect;
    }
};

static NullableRect rectUnion(const NullableRect &a, const NullableRect &b)
{
    Rect aRect = a.getRect();
    Rect bRect = b.getRect();

    if (a.isEmpty())
    {
        return b;
    }
    if (b.isEmpty())
    {
        return a;
    }

    int left = min(aRect.x, bRect.x);
    int top = min(aRect.y, bRect.y);
    int right = max(aRect.x + aRect.w, bRect.x + bRect.w);
    int bottom = max(aRect.y + aRect.h, bRect.y + bRect.h);
    return NullableRect(Rect{left, top, right - left, bottom - top});
}

static NullableRect rectIntersect(const Rect &a, const Rect &b)
{
    int left = max(a.x, b.x);
    int top = max(a.y, b.y);
    int right = min(a.x + a.w, b.x + b.w);
    int bottom = min(a.y + a.h, b.y + b.h);
    return NullableRect(Rect{left, top, right - left, bottom - top});
}

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

static constexpr int STRIP_ROWS = FB_HEIGHT / 10; // An arbitrary strip height; we can measure other sizes.

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

class Framebuffer
{
private:
    uint16_t *_pixels = nullptr; // in PSRAM, allocated once by init()
    NullableRect _dirtyRect;     // the changes not yet sent to the panel
    esp_panel::drivers::LCD *_lcd = nullptr;

    void setPixel(int x, int y, uint16_t color)
    {
        if (x < 0 || x >= FB_WIDTH || y < 0 || y >= FB_HEIGHT)
        {
            return;
        }
        _pixels[y * FB_WIDTH + x] = color;
    }

public:
    bool init(esp_panel::drivers::LCD *lcd)
    {
        _lcd = lcd;
        size_t bytes = FB_WIDTH * FB_HEIGHT * sizeof(uint16_t);
        _pixels = static_cast<uint16_t *>(ps_malloc(bytes));
        return _pixels != nullptr;
    }

    void markDirty(const Rect &r)
    {
        // Clamp the dirty rect to the framebuffer dimensions
        NullableRect visible = rectIntersect(r, Rect{0, 0, FB_WIDTH, FB_HEIGHT});
        _dirtyRect = rectUnion(_dirtyRect, visible);
    }

    void fillRect(const Rect &r, uint16_t color)
    {
        NullableRect visible = rectIntersect(r, Rect{0, 0, FB_WIDTH, FB_HEIGHT});

        if (visible.isEmpty())
        {
            return;
        }

        Rect visibleRect = visible.getRect();

        for (int row = visibleRect.y; row < visibleRect.y + visibleRect.h; row++)
        {
            for (int col = visibleRect.x; col < visibleRect.x + visibleRect.w; col++)
            {
                setPixel(col, row, color);
            }
        }
    }

    void drawSprite(int x, int y, int w, int h, const uint16_t *data, const Rect &clip)
    {
        NullableRect visible = rectIntersect(Rect{x, y, w, h}, clip);

        if (visible.isEmpty())
        {
            return;
        }

        Rect visibleRect = visible.getRect();
        for (int row = visibleRect.y; row < visibleRect.y + visibleRect.h; row++)
        {
            for (int col = visibleRect.x; col < visibleRect.x + visibleRect.w; col++)
            {
                uint16_t color = data[(row - y) * w + (col - x)];
                if (color != TRANSPARENT_COLOR)
                {
                    setPixel(col, row, color);
                }
            }
        }
    }

    bool isDirty() const
    {
        return !_dirtyRect.isEmpty();
    }

    Rect getDirtyRect() const
    {
        return _dirtyRect.getRect();
    }

    bool flush()
    {
        if (_dirtyRect.isEmpty())
        {
            return true;
        }

        for (int y = 0; y < FB_HEIGHT; y += STRIP_ROWS)
        {
            int rows = FB_HEIGHT - y;
            if (rows > STRIP_ROWS)
            {
                rows = STRIP_ROWS;
            }
            const uint8_t *data = reinterpret_cast<const uint8_t *>(&_pixels[y * FB_WIDTH]);
            if (!_lcd->drawBitmap(0, y, FB_WIDTH, rows, data, -1))
            {
                return false;
            }
        }

        _dirtyRect.clear();

        return true;
    }
};

static Framebuffer framebuffer;

static void renderRegion(const Rect &region)
{
    framebuffer.fillRect(region, background);
    framebuffer.drawSprite(spriteX, spriteY, SPRITE_BABY_WIDTH, SPRITE_BABY_HEIGHT, sprite_baby_data, region);
}

void setup()
{
    Serial.begin(115200);
    delay(5000); // Gives the serial monitor time to reconnect after the USB port resets.

    Board *board = new Board();
    board->init();
    bool ok = board->begin();
    assert(ok);

    esp_panel::drivers::LCD *lcd = board->getLCD();

    if (framebuffer.init(lcd))
    {
        framebuffer.markDirty(Rect{0, 0, FB_WIDTH, FB_HEIGHT});
        framebuffer.fillRect(Rect{0, 0, FB_WIDTH, FB_HEIGHT}, background);
        framebuffer.drawSprite(spriteX, spriteY, SPRITE_BABY_WIDTH, SPRITE_BABY_HEIGHT, sprite_baby_data, Rect{0, 0, FB_WIDTH, FB_HEIGHT});
        framebuffer.flush();
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
        framebuffer.markDirty(Rect{spriteX, spriteY, SPRITE_BABY_WIDTH, SPRITE_BABY_HEIGHT});
        spriteY--;
        framebuffer.markDirty(Rect{spriteX, spriteY, SPRITE_BABY_WIDTH, SPRITE_BABY_HEIGHT});
    }
    if (buttonStates[BUTTON_DOWN] == BUTTON_PRESSED)
    {
        framebuffer.markDirty(Rect{spriteX, spriteY, SPRITE_BABY_WIDTH, SPRITE_BABY_HEIGHT});
        spriteY++;
        framebuffer.markDirty(Rect{spriteX, spriteY, SPRITE_BABY_WIDTH, SPRITE_BABY_HEIGHT});
    }

    if (framebuffer.isDirty())
    {
        renderRegion(framebuffer.getDirtyRect());
        framebuffer.flush();
    }
}
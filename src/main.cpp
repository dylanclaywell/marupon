#include <Arduino.h>
#include <esp_display_panel.hpp>

#include "display_config.h"

#include "sprites/sprite_baby_cactus_32x32_1.h"
#include "geometry.h"
#include "panel_color.h"

using namespace esp_panel::board;

static constexpr int TE_PIN = 18; // Per the Waveshare wiki, this is the pin for the Tearing Effect signal

static volatile uint32_t tePulseCount = 0;
static uint32_t lastTickPulseCount = 0;
static uint32_t lastPrintedPulseCount = 0;
static constexpr int TE_TIMEOUT = 25; // Timeout for waiting for the next TE pulse, in milliseconds. This is used to prevent indefinite blocking if a TE pulse is missed, and also a fallback for slow or missing TE signals for updating the display
static int teTimeoutCount = 0;        // How many times the TE pulse wait has timed out

static unsigned long maxBitmapSendMicros = 0;
static unsigned long maxExpansionMicros = 0;

static void IRAM_ATTR onTePulse()
{
    tePulseCount++;
}

static constexpr uint16_t TRANSPARENT_COLOR = panelColor(255, 0, 255);

static constexpr uint16_t background = panelColor(0, 0, 64);

static constexpr int STAGING_PIXELS = 20000;
static_assert(PANEL_WIDTH * SCALE <= STAGING_PIXELS, "One expanded canvas row must fit in staging");

static int spriteX = CANVAS_WIDTH / 2 - SPRITE_BABY_CACTUS_32X32_1_WIDTH / 2;
static int spriteY = CANVAS_HEIGHT / 2 - SPRITE_BABY_CACTUS_32X32_1_HEIGHT / 2;

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

static unsigned long lastFpsPrint = 0;
static unsigned long lastTick = 0;

static Rect rectAlignToPanel(const Rect &r)
{
    int mask = PANEL_ALIGN - 1;
    int left = r.x & ~mask;
    int top = r.y & ~mask;
    int right = (r.x + r.w + mask) & ~mask;
    int bottom = (r.y + r.h + mask) & ~mask;
    return Rect{left, top, right - left, bottom - top};
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

static void waitForTePulse()
{
    uint32_t startPulseCount = tePulseCount;
    unsigned long startTime = millis();
    while (tePulseCount == startPulseCount)
    {
        if (millis() - startTime >= TE_TIMEOUT)
        {
            teTimeoutCount++;
            break; // Timeout reached, exit the wait loop
        }
    }
}

class Framebuffer
{
private:
    uint16_t *_canvasPixels = nullptr;      // in PSRAM, allocated once by init()
    uint16_t _panelStaging[STAGING_PIXELS]; // temporary buffer for sending rects to the LCD
    NullableRect _dirtyRect;                // the changes not yet sent to the panel
    esp_panel::drivers::LCD *_lcd = nullptr;

    void setPixel(int x, int y, uint16_t color)
    {
        if (x < 0 || x >= CANVAS_WIDTH || y < 0 || y >= CANVAS_HEIGHT)
        {
            return;
        }
        _canvasPixels[y * CANVAS_WIDTH + x] = color;
    }

public:
    bool init(esp_panel::drivers::LCD *lcd)
    {
        _lcd = lcd;
        size_t bytes = CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(uint16_t);
        _canvasPixels = static_cast<uint16_t *>(ps_malloc(bytes));
        return _canvasPixels != nullptr;
    }

    void markDirty(const Rect &r)
    {
        // Clamp the dirty rect to the framebuffer dimensions
        NullableRect visible = rectIntersect(r, Rect{0, 0, CANVAS_WIDTH, CANVAS_HEIGHT});
        _dirtyRect = rectUnion(_dirtyRect, NullableRect(rectAlignToPanel(visible.getRect())));
    }

    void fillRect(const Rect &r, uint16_t color)
    {
        NullableRect visible = rectIntersect(r, Rect{0, 0, CANVAS_WIDTH, CANVAS_HEIGHT});

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

        Rect canvasRect = _dirtyRect.getRect();
        Rect panelRect = Rect{canvasRect.x * SCALE, canvasRect.y * SCALE, canvasRect.w * SCALE, canvasRect.h * SCALE};

        int panelRowWidth = panelRect.w;   // pixels across one panel row
        int panelRowsPerCanvasRow = SCALE; // each canvas row becomes this many panel rows
        int stagingPixelsPerCanvasRow = panelRowWidth * panelRowsPerCanvasRow;

        int maxRowsPerBand = STAGING_PIXELS / stagingPixelsPerCanvasRow;

        for (int bandStartRow = 0; bandStartRow < canvasRect.h; bandStartRow += maxRowsPerBand)
        {
            int rowsLeft = canvasRect.h - bandStartRow;
            int rowsThisBand = maxRowsPerBand; // Default to a full band
            if (rowsLeft < maxRowsPerBand)
            {
                rowsThisBand = rowsLeft; // Use the remaining rows if fewer than a full band
            }

            unsigned long expansionStartMicros = micros();
            for (int row = 0; row < rowsThisBand; row++)
            {
                int canvasRow = canvasRect.y + bandStartRow + row;
                const uint16_t *srcRow = &_canvasPixels[canvasRow * CANVAS_WIDTH + canvasRect.x];
                uint16_t *destRow = &_panelStaging[row * SCALE * panelRect.w];

                for (int canvasCol = 0; canvasCol < canvasRect.w; canvasCol++)
                {
                    for (int colRepeat = 0; colRepeat < SCALE; colRepeat++)
                    {
                        destRow[canvasCol * SCALE + colRepeat] = srcRow[canvasCol];
                    }
                }

                // Duplicate the first row SCALE-1 times to achieve vertical scaling (skipping the first row which is already copied)
                for (int rowRepeat = 1; rowRepeat < SCALE; rowRepeat++)
                {
                    memcpy(&destRow[rowRepeat * panelRect.w], &destRow[0], canvasRect.w * SCALE * sizeof(uint16_t));
                }
            }
            unsigned long expansionEndMicros = micros();
            unsigned long expansionMicros = expansionEndMicros - expansionStartMicros;
            if (expansionMicros > maxExpansionMicros)
            {
                maxExpansionMicros = expansionMicros;
            }

            // Wait for the next TE pulse before drawing the next band (if using TE synchronization).
            // We only wait for the TE pulse before drawing the first band. Subsequent bands are drawn immediately.
            if (bandStartRow == 0)
            {
                waitForTePulse();
            }

            unsigned long startMicros = micros();
            if (!_lcd->drawBitmap(panelRect.x, panelRect.y + (bandStartRow * SCALE), panelRect.w, rowsThisBand * SCALE,
                                  reinterpret_cast<const uint8_t *>(_panelStaging), -1))
            {
                return false;
            }
            unsigned long endMicros = micros();
            unsigned long bitmapSendMicros = endMicros - startMicros;
            if (bitmapSendMicros > maxBitmapSendMicros)
            {
                maxBitmapSendMicros = bitmapSendMicros;
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
    framebuffer.drawSprite(spriteX, spriteY, SPRITE_BABY_CACTUS_32X32_1_WIDTH, SPRITE_BABY_CACTUS_32X32_1_HEIGHT, sprite_baby_cactus_32x32_1_data, region);
}

static void process()
{
    if (buttonStates[BUTTON_UP] == BUTTON_PRESSED)
    {
        framebuffer.markDirty(Rect{spriteX, spriteY, SPRITE_BABY_CACTUS_32X32_1_WIDTH, SPRITE_BABY_CACTUS_32X32_1_HEIGHT});
        spriteY--;
        framebuffer.markDirty(Rect{spriteX, spriteY, SPRITE_BABY_CACTUS_32X32_1_WIDTH, SPRITE_BABY_CACTUS_32X32_1_HEIGHT});
    }
    if (buttonStates[BUTTON_DOWN] == BUTTON_PRESSED)
    {
        framebuffer.markDirty(Rect{spriteX, spriteY, SPRITE_BABY_CACTUS_32X32_1_WIDTH, SPRITE_BABY_CACTUS_32X32_1_HEIGHT});
        spriteY++;
        framebuffer.markDirty(Rect{spriteX, spriteY, SPRITE_BABY_CACTUS_32X32_1_WIDTH, SPRITE_BABY_CACTUS_32X32_1_HEIGHT});
    }
}

void setup()
{
    Serial.begin(115200);
    delay(5000); // Gives the serial monitor time to reconnect after the USB port resets.

    Board *board = new Board();
    board->init();
    bool ok = board->begin();
    assert(ok);

    pinMode(TE_PIN, INPUT);
    attachInterrupt(TE_PIN, onTePulse, RISING);

    esp_panel::drivers::LCD *lcd = board->getLCD();

    if (framebuffer.init(lcd))
    {
        framebuffer.markDirty(Rect{0, 0, CANVAS_WIDTH, CANVAS_HEIGHT});
        framebuffer.fillRect(Rect{0, 0, CANVAS_WIDTH, CANVAS_HEIGHT}, background);
        framebuffer.drawSprite(spriteX, spriteY, SPRITE_BABY_CACTUS_32X32_1_WIDTH, SPRITE_BABY_CACTUS_32X32_1_HEIGHT, sprite_baby_cactus_32x32_1_data, Rect{0, 0, CANVAS_WIDTH, CANVAS_HEIGHT});
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

    lastTick = millis();
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

    unsigned long now = millis();
    uint32_t currentPulseCount = tePulseCount;
    if (currentPulseCount != lastTickPulseCount || now - lastTick >= TE_TIMEOUT)
    {
        lastTickPulseCount = currentPulseCount;
        lastTick = now;
        process();
    }

    if (framebuffer.isDirty())
    {
        renderRegion(framebuffer.getDirtyRect());
        framebuffer.flush();
    }

    // print FPS every second instead of every frame
    if (now - lastFpsPrint >= 1000)
    {
        lastFpsPrint = now;

        uint32_t totalPulseCount = tePulseCount;
        uint32_t pulsesThisSecond = totalPulseCount - lastPrintedPulseCount;
        lastPrintedPulseCount = totalPulseCount;

        Serial.printf(
            "te/s: %lu; teTimeoutCount: %d; maxBitmapSendMicros: %lu; maxExpansionMicros: %lu;\n",
            pulsesThisSecond,
            teTimeoutCount,
            maxBitmapSendMicros,
            maxExpansionMicros);
        teTimeoutCount = 0;
        maxBitmapSendMicros = 0;
        maxExpansionMicros = 0;
    }
}
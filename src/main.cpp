#include <Arduino.h>
#include <esp_display_panel.hpp>

#include "display_config.h"

#include "geometry.h"
#include "panel_color.h"
#include "te_sync.h"
#include "buttons.h"
#include "game/game.h"

using namespace esp_panel::board;

static uint32_t lastTickPulseCount = 0;
static uint32_t lastPrintedPulseCount = 0;

static uint32_t lastPrintedTimeoutCount = 0;

static unsigned long lastFpsPrint = 0;
static unsigned long lastTick = 0;

static Game game;

void setup()
{
    Serial.begin(115200);
    delay(5000); // Gives the serial monitor time to reconnect after the USB port resets.

    Board *board = new Board();
    board->init();
    bool ok = board->begin();
    assert(ok);

    teSyncBegin();

    esp_panel::drivers::LCD *lcd = board->getLCD();

    if (!game.init(lcd))
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
    pollButtonsFromSerial();

    unsigned long now = millis();
    uint32_t currentPulseCount = getTePulseCount();
    if (currentPulseCount != lastTickPulseCount || now - lastTick >= TE_TIMEOUT)
    {
        lastTickPulseCount = currentPulseCount;
        lastTick = now;
        game.process();
    }

    game.redraw();

    // print TE pulse and timeout statistics every second
    if (now - lastFpsPrint >= 1000)
    {
        lastFpsPrint = now;

        uint32_t totalPulseCount = getTePulseCount();
        uint32_t pulsesThisSecond = totalPulseCount - lastPrintedPulseCount;
        lastPrintedPulseCount = totalPulseCount;

        uint32_t totalTimeoutCount = getTeTimeoutCount();
        uint32_t timeoutsThisSecond = totalTimeoutCount - lastPrintedTimeoutCount;
        lastPrintedTimeoutCount = totalTimeoutCount;

        Serial.printf(
            "te/s: %lu; teTimeoutCount: %lu; maxBitmapSendMicros: %lu; maxExpansionMicros: %lu;\n",
            pulsesThisSecond,
            timeoutsThisSecond,
            game.getFramebufferDebugStats().maxBitmapSendMicros,
            game.getFramebufferDebugStats().maxExpansionMicros);

        game.resetFramebufferDebugStats();
    }
}
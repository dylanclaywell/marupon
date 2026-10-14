#pragma once

#include <stdint.h>

#include "display_config.h"
#include "geometry.h"

// Forward declare esp_panel::drivers::LCD so that we don't include the full esp_display_panel.hpp header here as it's not needed.
namespace esp_panel::drivers
{
  class LCD;
}

struct DebugStats
{
  unsigned long maxBitmapSendMicros = 0;
  unsigned long maxExpansionMicros = 0;
};

class Framebuffer
{
private:
  uint16_t *_canvasPixels = nullptr;      // in PSRAM, allocated once by init()
  uint16_t _panelStaging[STAGING_PIXELS]; // temporary buffer for sending rects to the LCD
  NullableRect _dirtyRect;                // the changes not yet sent to the panel
  esp_panel::drivers::LCD *_lcd = nullptr;
  DebugStats _debugStats;

  void setPixel(int x, int y, uint16_t color);

  static Rect rectAlignToPanel(const Rect &r);

public:
  bool init(esp_panel::drivers::LCD *lcd);
  void markDirty(const Rect &r);
  void fillRect(const Rect &r, uint16_t color);
  void drawSprite(int x, int y, int w, int h, const uint16_t *data, const Rect &clip);
  bool flush();
  void resetDebugStats();

  bool isDirty() const
  {
    return !_dirtyRect.isEmpty();
  }

  Rect getDirtyRect() const
  {
    return _dirtyRect.getRect();
  }

  DebugStats getDebugStats() const
  {
    return _debugStats;
  }
};
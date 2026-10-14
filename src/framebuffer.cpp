#include <Arduino.h>
#include <esp_display_panel.hpp>

#include "framebuffer.h"
#include "te_sync.h"
#include "display_config.h"

Rect Framebuffer::rectAlignToPanel(const Rect &r)
{
  int mask = PANEL_ALIGN - 1;
  int left = r.x & ~mask;
  int top = r.y & ~mask;
  int right = (r.x + r.w + mask) & ~mask;
  int bottom = (r.y + r.h + mask) & ~mask;
  return Rect{left, top, right - left, bottom - top};
}

void Framebuffer::setPixel(int x, int y, uint16_t color)
{
  if (x < 0 || x >= CANVAS_WIDTH || y < 0 || y >= CANVAS_HEIGHT)
  {
    return;
  }
  _canvasPixels[y * CANVAS_WIDTH + x] = color;
}

bool Framebuffer::init(esp_panel::drivers::LCD *lcd)
{
  _lcd = lcd;
  size_t bytes = CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(uint16_t);
  _canvasPixels = static_cast<uint16_t *>(ps_malloc(bytes));
  return _canvasPixels != nullptr;
}

void Framebuffer::markDirty(const Rect &r)
{
  // Clamp the dirty rect to the framebuffer dimensions
  NullableRect visible = rectIntersect(r, Rect{0, 0, CANVAS_WIDTH, CANVAS_HEIGHT});
  _dirtyRect = rectUnion(_dirtyRect, NullableRect(rectAlignToPanel(visible.getRect())));
}

void Framebuffer::fillRect(const Rect &r, uint16_t color)
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

void Framebuffer::drawSprite(int x, int y, int w, int h, const uint16_t *data, const Rect &clip)
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

bool Framebuffer::flush()
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
    if (expansionMicros > _debugStats.maxExpansionMicros)
    {
      _debugStats.maxExpansionMicros = expansionMicros;
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
    if (bitmapSendMicros > _debugStats.maxBitmapSendMicros)
    {
      _debugStats.maxBitmapSendMicros = bitmapSendMicros;
    }
  }

  _dirtyRect.clear();

  return true;
}

void Framebuffer::resetDebugStats()
{
  _debugStats.maxBitmapSendMicros = 0;
  _debugStats.maxExpansionMicros = 0;
}

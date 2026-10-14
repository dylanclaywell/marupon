#pragma once
#include "framebuffer.h"
#include "display_config.h"

#include "sprites/sprite_baby_cactus_32x32_1.h"

// Forward declare esp_panel::drivers::LCD so that we don't include the full esp_display_panel.hpp header here as it's not needed.
namespace esp_panel::drivers
{
  class LCD;
}

class Game
{
private:
  static constexpr uint16_t background = panelColor(0, 0, 64);

  Framebuffer framebuffer;
  int spriteX = CANVAS_WIDTH / 2 - SPRITE_BABY_CACTUS_32X32_1_WIDTH / 2;
  int spriteY = CANVAS_HEIGHT / 2 - SPRITE_BABY_CACTUS_32X32_1_HEIGHT / 2;

  void render(const Rect &region);

public:
  void process();
  void redraw();
  bool init(esp_panel::drivers::LCD *lcd);

  DebugStats getFramebufferDebugStats() const
  {
    return framebuffer.getDebugStats();
  }

  void resetFramebufferDebugStats()
  {
    framebuffer.resetDebugStats();
  }
};
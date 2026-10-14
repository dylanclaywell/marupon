#include "game.h"
#include "buttons.h"

void Game::process()
{
  if (getButtonState(BUTTON_UP) == BUTTON_PRESSED)
  {
    framebuffer.markDirty(Rect{spriteX, spriteY, SPRITE_BABY_CACTUS_32X32_1_WIDTH, SPRITE_BABY_CACTUS_32X32_1_HEIGHT});
    spriteY--;
    framebuffer.markDirty(Rect{spriteX, spriteY, SPRITE_BABY_CACTUS_32X32_1_WIDTH, SPRITE_BABY_CACTUS_32X32_1_HEIGHT});
  }
  if (getButtonState(BUTTON_DOWN) == BUTTON_PRESSED)
  {
    framebuffer.markDirty(Rect{spriteX, spriteY, SPRITE_BABY_CACTUS_32X32_1_WIDTH, SPRITE_BABY_CACTUS_32X32_1_HEIGHT});
    spriteY++;
    framebuffer.markDirty(Rect{spriteX, spriteY, SPRITE_BABY_CACTUS_32X32_1_WIDTH, SPRITE_BABY_CACTUS_32X32_1_HEIGHT});
  }
}

void Game::render(const Rect &region)
{
  framebuffer.fillRect(region, background);
  framebuffer.drawSprite(spriteX, spriteY, SPRITE_BABY_CACTUS_32X32_1_WIDTH, SPRITE_BABY_CACTUS_32X32_1_HEIGHT, sprite_baby_cactus_32x32_1_data, region);
}

void Game::redraw()
{
  if (framebuffer.isDirty())
  {
    render(framebuffer.getDirtyRect());
    framebuffer.flush();
  }
}

bool Game::init(esp_panel::drivers::LCD *lcd)
{
  if (framebuffer.init(lcd))
  {
    framebuffer.markDirty(Rect{0, 0, CANVAS_WIDTH, CANVAS_HEIGHT});
    framebuffer.fillRect(Rect{0, 0, CANVAS_WIDTH, CANVAS_HEIGHT}, background);
    framebuffer.drawSprite(spriteX, spriteY, SPRITE_BABY_CACTUS_32X32_1_WIDTH, SPRITE_BABY_CACTUS_32X32_1_HEIGHT, sprite_baby_cactus_32x32_1_data, Rect{0, 0, CANVAS_WIDTH, CANVAS_HEIGHT});
    framebuffer.flush();
    return true;
  }
  return false;
}
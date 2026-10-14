#pragma once

#include "panel_color.h"

constexpr int SCALE = 4;
static_assert(4 % SCALE == 0, "SCALE must evenly divide 4");

constexpr int PANEL_ALIGN = 4 / SCALE; // The panel wants alignment to multiples of 4 pixels, and we adjust for the current scale

constexpr int PANEL_WIDTH = 412;
constexpr int PANEL_HEIGHT = 412;
static_assert(PANEL_WIDTH % SCALE == 0 && PANEL_HEIGHT % SCALE == 0, "Panel dimensions must be divisible by SCALE");

constexpr int CANVAS_WIDTH = PANEL_WIDTH / SCALE;
constexpr int CANVAS_HEIGHT = PANEL_HEIGHT / SCALE;
static_assert(CANVAS_WIDTH % PANEL_ALIGN == 0 && CANVAS_HEIGHT % PANEL_ALIGN == 0, "Canvas dimensions must be divisible by PANEL_ALIGN");

static constexpr uint16_t TRANSPARENT_COLOR = panelColor(255, 0, 255);

static constexpr int STAGING_PIXELS = 20000;
static_assert(PANEL_WIDTH * SCALE <= STAGING_PIXELS, "One expanded canvas row must fit in staging");

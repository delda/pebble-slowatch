#pragma once

#include <pebble.h>

#include "../clock.h"

typedef enum {
  CLASSIC_TICKS_STANDARD,
  CLASSIC_TICKS_SPLIT,
  CLASSIC_TICKS_SHORT,
} ClassicTickStyle;

void classic_layout_init(void);
void classic_layout_deinit(void);

GColor classic_layout_background_color(void);
void classic_layout_draw(Layer *layer, GContext *ctx, ClockTime time, ClassicTickStyle tick_style);

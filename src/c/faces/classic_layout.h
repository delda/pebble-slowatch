#pragma once

#include <pebble.h>

#include "../clock.h"

void classic_layout_init(void);
void classic_layout_deinit(void);

GColor classic_layout_background_color(void);
void classic_layout_draw(Layer *layer, GContext *ctx, ClockTime time, bool split_long_ticks);

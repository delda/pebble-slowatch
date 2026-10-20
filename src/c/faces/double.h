#pragma once

#include <pebble.h>

#include "../clock.h"

GColor double_face_background_color(void);
void double_face_draw(Layer *layer, GContext *ctx, ClockTime time);

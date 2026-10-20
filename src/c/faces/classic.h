#pragma once

#include <pebble.h>

#include "../clock.h"

GColor classic_face_background_color(void);
void classic_face_draw(Layer *layer, GContext *ctx, ClockTime time);

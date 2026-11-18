#pragma once

#include <pebble.h>
#include "../clock.h"

GColor gold_ring_face_background_color(void);
void gold_ring_face_draw(Layer *layer, GContext *ctx, ClockTime time);

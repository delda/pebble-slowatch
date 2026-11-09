#pragma once

#include <pebble.h>
#include "../clock.h"

void perimeter_draw_hand(GContext *ctx, GRect bounds, ClockTime time);

void perimeter_face_draw(Layer *layer, GContext *ctx, ClockTime time);

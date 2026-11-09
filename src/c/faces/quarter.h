#pragma once

#include <pebble.h>

#include "../clock.h"

void quarter_face_draw(Layer *layer, GContext *ctx, ClockTime time);

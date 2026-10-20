#pragma once

#include <pebble.h>

#include "clock.h"

void watch_face_init(void);
void watch_face_deinit(void);

GColor watch_face_background_color(void);
void watch_face_draw(Layer *layer, GContext *ctx, ClockTime time);

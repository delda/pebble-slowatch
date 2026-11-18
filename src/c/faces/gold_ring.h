#pragma once

#include <pebble.h>
#include "../clock.h"

#if defined(PBL_PLATFORM_GABBRO) || defined(PBL_PLATFORM_CHALK) || defined(PBL_PLATFORM_EMERY)
GColor gold_ring_face_background_color(void);
void gold_ring_face_draw(Layer *layer, GContext *ctx, ClockTime time);
#endif

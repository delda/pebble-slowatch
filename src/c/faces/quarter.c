#include "quarter.h"

#include "classic_layout.h"

void quarter_face_draw(Layer *layer, GContext *ctx, ClockTime time) {
  classic_layout_draw(layer, ctx, time, CLASSIC_TICKS_SHORT);
}

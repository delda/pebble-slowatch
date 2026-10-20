#include "classic.h"

#include "classic_layout.h"

GColor classic_face_background_color(void) {
  return classic_layout_background_color();
}

void classic_face_draw(Layer *layer, GContext *ctx, ClockTime time) {
  classic_layout_draw(layer, ctx, time, false);
}

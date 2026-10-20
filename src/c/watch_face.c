#include "watch_face.h"

#include "faces/classic.h"
#include "settings.h"

GColor watch_face_background_color(void) {
  switch (settings_dial()) {
    case DIAL_CLASSIC:
    default:
      return classic_face_background_color();
  }
}

void watch_face_draw(Layer *layer, GContext *ctx, ClockTime time) {
  switch (settings_dial()) {
    case DIAL_CLASSIC:
    default:
      classic_face_draw(layer, ctx, time);
      break;
  }
}

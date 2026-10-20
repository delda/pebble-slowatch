#include "watch_face.h"

#include "faces/classic.h"
#include "faces/classic_layout.h"
#include "faces/double.h"
#include "settings.h"

void watch_face_init(void) {
  classic_layout_init();
}

void watch_face_deinit(void) {
  classic_layout_deinit();
}

GColor watch_face_background_color(void) {
  switch (settings_dial()) {
    case DIAL_DOUBLE:
      return double_face_background_color();
    case DIAL_CLASSIC:
    default:
      return classic_face_background_color();
  }
}

void watch_face_draw(Layer *layer, GContext *ctx, ClockTime time) {
  switch (settings_dial()) {
    case DIAL_DOUBLE:
      double_face_draw(layer, ctx, time);
      break;
    case DIAL_CLASSIC:
    default:
      classic_face_draw(layer, ctx, time);
      break;
  }
}

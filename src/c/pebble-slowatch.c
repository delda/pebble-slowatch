#include <pebble.h>

#include "clock.h"
#include "watch_face.h"

static Window *s_window;
static Layer *s_face_layer;

static void prv_face_layer_update(Layer *layer, GContext *ctx) {
  watch_face_draw(layer, ctx, clock_current_time());
}

static void prv_clock_updated(void) {
  layer_mark_dirty(s_face_layer);
}

static void prv_window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  s_face_layer = layer_create(layer_get_bounds(root_layer));
  layer_set_update_proc(s_face_layer, prv_face_layer_update);
  layer_add_child(root_layer, s_face_layer);
}

static void prv_window_unload(Window *window) {
  layer_destroy(s_face_layer);
}

static void prv_init(void) {
  s_window = window_create();
  window_set_background_color(s_window, watch_face_background_color());
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);
  clock_init(prv_clock_updated);
}

static void prv_deinit(void) {
  clock_deinit();
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}

#include <pebble.h>

#include "clock.h"
#include "settings.h"
#include "watch_face.h"

static Window *s_window;
static Layer *s_face_layer;

static void prv_face_layer_update(Layer *layer, GContext *ctx) {
  watch_face_draw(layer, ctx, clock_current_time());
}

static void prv_clock_updated(void) {
  layer_mark_dirty(s_face_layer);
}

static void prv_inbox_received(DictionaryIterator *iterator, void *context) {
  Tuple *position_tuple = dict_find(iterator, MESSAGE_KEY_midnight_position);
  if (position_tuple == NULL || position_tuple->type != TUPLE_CSTRING) {
    return;
  }

  if (strcmp(position_tuple->value->cstring, "bottom") == 0) {
    settings_set_midnight_position(MIDNIGHT_POSITION_BOTTOM);
  } else if (strcmp(position_tuple->value->cstring, "top") == 0) {
    settings_set_midnight_position(MIDNIGHT_POSITION_TOP);
  }
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
  settings_init(prv_clock_updated);
  app_message_register_inbox_received(prv_inbox_received);
  app_message_open(128, 64);
}

static void prv_deinit(void) {
  settings_deinit();
  clock_deinit();
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}

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
  Tuple *layout_tuple = dict_find(iterator, MESSAGE_KEY_color_layout);
  if (layout_tuple != NULL && layout_tuple->type == TUPLE_CSTRING) {
    const char *layout = layout_tuple->value->cstring;
    if (strcmp(layout, "default") == 0) settings_set_color_layout(COLOR_LAYOUT_DEFAULT);
    else if (strcmp(layout, "snow") == 0) settings_set_color_layout(COLOR_LAYOUT_SNOW);
    else if (strcmp(layout, "night") == 0) settings_set_color_layout(COLOR_LAYOUT_NIGHT);
    else if (strcmp(layout, "sand") == 0) settings_set_color_layout(COLOR_LAYOUT_SAND);
    else if (strcmp(layout, "sky") == 0) settings_set_color_layout(COLOR_LAYOUT_SKY);
    else if (strcmp(layout, "vanilla") == 0) settings_set_color_layout(COLOR_LAYOUT_VANILLA);
    else if (strcmp(layout, "sun") == 0) settings_set_color_layout(COLOR_LAYOUT_SUN);
    else if (strcmp(layout, "meadow") == 0) settings_set_color_layout(COLOR_LAYOUT_MEADOW);
    else if (strcmp(layout, "mint") == 0) settings_set_color_layout(COLOR_LAYOUT_MINT);
    else if (strcmp(layout, "forest") == 0) settings_set_color_layout(COLOR_LAYOUT_FOREST);
  }

  Tuple *position_tuple = dict_find(iterator, MESSAGE_KEY_midnight_position);
  if (position_tuple != NULL && position_tuple->type == TUPLE_CSTRING) {
    if (strcmp(position_tuple->value->cstring, "bottom") == 0) {
      settings_set_midnight_position(MIDNIGHT_POSITION_BOTTOM);
    } else if (strcmp(position_tuple->value->cstring, "top") == 0) {
      settings_set_midnight_position(MIDNIGHT_POSITION_TOP);
    }
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
  settings_init(prv_clock_updated);
  s_window = window_create();
  window_set_background_color(s_window, watch_face_background_color());
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);
  clock_init(prv_clock_updated);
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

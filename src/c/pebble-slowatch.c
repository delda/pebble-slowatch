#include <pebble.h>

#define TICKS_PER_HOUR 4

static Window *s_window;
static Layer *s_face_layer;

static GColor prv_navy(void) {
  return GColorFromHEX(0x062D54);
}

static GColor prv_copper(void) {
  return GColorFromHEX(0xC98858);
}

static GPoint prv_point_on_circle(GPoint centre, int16_t radius, int32_t angle) {
  return GPoint(centre.x + (int16_t)(sin_lookup(angle) * radius / TRIG_MAX_RATIO),
                centre.y - (int16_t)(cos_lookup(angle) * radius / TRIG_MAX_RATIO));
}

static void prv_draw_tick(GContext *ctx, GPoint centre, int16_t radius,
                          int32_t angle, bool half_hour_tick) {
  const int16_t length = half_hour_tick ? 9 : 4;
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, half_hour_tick ? 2 : 1);
  graphics_draw_line(ctx, prv_point_on_circle(centre, radius, angle),
                     prv_point_on_circle(centre, radius - length, angle));
}

static void prv_face_layer_update(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint centre = grect_center_point(&bounds);
  const int16_t smallest_side = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
  const int16_t outer_radius = smallest_side / 2 - 5;
  const int16_t tick_radius = outer_radius - 5;
  const int16_t label_radius = tick_radius - 12;

  graphics_context_set_fill_color(ctx, prv_navy());
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, centre, outer_radius);

  for (int tick = 0; tick < HOURS_PER_DAY * TICKS_PER_HOUR; tick++) {
    const bool half_hour_tick = (tick % TICKS_PER_HOUR) == TICKS_PER_HOUR / 2;
    prv_draw_tick(ctx, centre, tick_radius,
                  tick * TRIG_MAX_ANGLE / (HOURS_PER_DAY * TICKS_PER_HOUR), half_hour_tick);
  }

  graphics_context_set_text_color(ctx, GColorWhite);
  for (int hour = 1; hour <= HOURS_PER_DAY; hour++) {
    const int32_t angle = hour * TRIG_MAX_ANGLE / HOURS_PER_DAY;
    GPoint point = prv_point_on_circle(centre, label_radius, angle);
    char label[3];
    snprintf(label, sizeof(label), "%d", hour);
    GRect label_box = GRect(point.x - 10, point.y - 8, 20, 16);
    graphics_draw_text(ctx, label, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                       label_box, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }

  time_t now = time(NULL);
  struct tm *time_info = localtime(&now);
  const int32_t minutes_today = time_info->tm_hour * 60 + time_info->tm_min;
  const int32_t hand_angle = minutes_today * TRIG_MAX_ANGLE / (HOURS_PER_DAY * 60);
  const GPoint hand_end = prv_point_on_circle(centre, tick_radius, hand_angle);
  const GPoint tail_end = prv_point_on_circle(centre, 20, hand_angle + TRIG_MAX_ANGLE / 2);

  graphics_context_set_stroke_color(ctx, prv_copper());
  graphics_context_set_stroke_width(ctx, 4);
  graphics_draw_line(ctx, tail_end, hand_end);
  graphics_context_set_fill_color(ctx, prv_copper());
  graphics_fill_circle(ctx, centre, 8);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, centre, 8);
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
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
  window_set_background_color(s_window, prv_navy());
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
}

static void prv_deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}

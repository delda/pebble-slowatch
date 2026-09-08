#include "watch_face.h"

#define TICKS_PER_HOUR 4

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

static void prv_draw_dial(GContext *ctx, GRect bounds, ClockTime time,
                          int16_t edge_inset, int16_t label_inset) {
  GPoint centre = grect_center_point(&bounds);
  const int16_t smallest_side = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
  const int16_t outer_radius = smallest_side / 2 - edge_inset;
  const int16_t tick_radius = outer_radius - 5;
  const int16_t label_radius = tick_radius - label_inset;

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
    graphics_draw_text(ctx, label, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                       GRect(point.x - 10, point.y - 8, 20, 16),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }

  const int32_t hand_angle = time.minutes_today * TRIG_MAX_ANGLE / (HOURS_PER_DAY * 60);
  graphics_context_set_stroke_color(ctx, prv_copper());
  graphics_context_set_stroke_width(ctx, 4);
  graphics_draw_line(ctx, prv_point_on_circle(centre, 20, hand_angle + TRIG_MAX_ANGLE / 2),
                     prv_point_on_circle(centre, tick_radius, hand_angle));
  graphics_context_set_fill_color(ctx, prv_copper());
  graphics_fill_circle(ctx, centre, 8);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, centre, 8);
}

#if defined(PBL_PLATFORM_CHALK)
static void prv_draw_chalk_face(GContext *ctx, GRect bounds, ClockTime time) {
  prv_draw_dial(ctx, bounds, time, 5, 12);
}
#endif

#if defined(PBL_PLATFORM_GABBRO)
static void prv_draw_gabbro_face(GContext *ctx, GRect bounds, ClockTime time) {
  prv_draw_dial(ctx, bounds, time, 8, 15);
}
#endif

GColor watch_face_background_color(void) {
  return prv_navy();
}

void watch_face_draw(Layer *layer, GContext *ctx, ClockTime time) {
  const GRect bounds = layer_get_bounds(layer);

#if defined(PBL_PLATFORM_CHALK)
  prv_draw_chalk_face(ctx, bounds, time);
#elif defined(PBL_PLATFORM_GABBRO)
  prv_draw_gabbro_face(ctx, bounds, time);
#else
  prv_draw_dial(ctx, bounds, time, 5, 12);
#endif
}

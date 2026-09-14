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

#if !defined(PBL_PLATFORM_FLINT) && !defined(PBL_PLATFORM_DIORITE)
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
#endif

#if defined(PBL_PLATFORM_FLINT) || defined(PBL_PLATFORM_DIORITE)
static GPoint prv_point_on_rectangle(GPoint centre, int16_t half_width,
                                     int16_t half_height, int32_t angle) {
  const int32_t sine = sin_lookup(angle);
  const int32_t cosine = cos_lookup(angle);
  const int32_t absolute_sine = sine < 0 ? -sine : sine;
  const int32_t absolute_cosine = cosine < 0 ? -cosine : cosine;

  if (absolute_sine * half_height > absolute_cosine * half_width) {
    return GPoint(centre.x + sine * half_width / absolute_sine,
                  centre.y - cosine * half_width / absolute_sine);
  }

  return GPoint(centre.x + sine * half_height / absolute_cosine,
                centre.y - cosine * half_height / absolute_cosine);
}

static GPoint prv_flint_label_offset(int hour) {
  switch (hour) {
    case 13:
      return GPoint(3, 0);
    case 14:
      return GPoint(5, 0);
    case 15:
      return GPoint(3, -10);
    case 16:
      return GPoint(0, -5);
    case 17:
      return GPoint(0, -3);
    case 18:
      return GPoint(0, -1);
    case 19:
      return GPoint(0, 1);
    case 20:
      return GPoint(0, 5);
    case 21:
      return GPoint(4, 10);
    case 22:
      return GPoint(5, 0);
    case 23:
      return GPoint(3, 0);
  }

  if (hour >= 1 && hour <= 11) {
    const GPoint opposite_offset = prv_flint_label_offset(hour + 12);
    return GPoint(-opposite_offset.x, -opposite_offset.y);
  }

  return GPoint(0, 0);
}

static void prv_draw_flint_label(GContext *ctx, int hour, GPoint point) {
  char label[3];
  snprintf(label, sizeof(label), "%d", hour);
  graphics_draw_text(ctx, label, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                     GRect(point.x - 10, point.y - 8, 20, 16),
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void prv_draw_flint_face(GContext *ctx, GRect bounds, ClockTime time) {
  const int16_t label_inset = 15;
  const int16_t smallest_side = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
  const int16_t hand_radius = smallest_side / 2 - 28;
  const GPoint centre = grect_center_point(&bounds);
  const int16_t label_half_width = centre.x - label_inset;
  const int16_t label_half_height = centre.y - label_inset;
  const int16_t tick_half_width = centre.x - 2;
  const int16_t tick_half_height = centre.y - 2;

  graphics_context_set_fill_color(ctx, prv_navy());
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_rect(ctx, GRect(1, 1, bounds.size.w - 2, bounds.size.h - 2));
  graphics_context_set_text_color(ctx, GColorWhite);

  for (int tick = 0; tick < HOURS_PER_DAY * TICKS_PER_HOUR; tick++) {
    const int32_t angle = tick * TRIG_MAX_ANGLE / (HOURS_PER_DAY * TICKS_PER_HOUR);
    const bool half_hour_tick = (tick % TICKS_PER_HOUR) == TICKS_PER_HOUR / 2;
    const int16_t tick_length = half_hour_tick ? 9 : 4;
    const GPoint outer_point = prv_point_on_rectangle(centre, tick_half_width,
                                                       tick_half_height, angle);
    const GPoint inner_point = prv_point_on_rectangle(centre, tick_half_width - tick_length,
                                                       tick_half_height - tick_length, angle);
    graphics_context_set_stroke_width(ctx, half_hour_tick ? 2 : 1);
    graphics_draw_line(ctx, outer_point, inner_point);
  }

  for (int hour = 1; hour <= HOURS_PER_DAY; hour++) {
    const int32_t previous_tick_angle =
        (hour * TICKS_PER_HOUR - TICKS_PER_HOUR / 2) * TRIG_MAX_ANGLE /
        (HOURS_PER_DAY * TICKS_PER_HOUR);
    int32_t next_tick_angle =
        (hour * TICKS_PER_HOUR + TICKS_PER_HOUR / 2) * TRIG_MAX_ANGLE /
        (HOURS_PER_DAY * TICKS_PER_HOUR);
    if (next_tick_angle >= TRIG_MAX_ANGLE) {
      next_tick_angle -= TRIG_MAX_ANGLE;
    }
    const GPoint previous_tick = prv_point_on_rectangle(centre, tick_half_width,
                                                         tick_half_height,
                                                         previous_tick_angle);
    const GPoint next_tick = prv_point_on_rectangle(centre, tick_half_width,
                                                     tick_half_height, next_tick_angle);
    const GPoint tick_midpoint = GPoint((previous_tick.x + next_tick.x) / 2,
                                        (previous_tick.y + next_tick.y) / 2);
    const int16_t midpoint_x_offset = tick_midpoint.x - centre.x;
    const int16_t midpoint_y_offset = tick_midpoint.y - centre.y;
    const int16_t absolute_x_offset = midpoint_x_offset < 0 ? -midpoint_x_offset : midpoint_x_offset;
    const int16_t absolute_y_offset = midpoint_y_offset < 0 ? -midpoint_y_offset : midpoint_y_offset;
    GPoint label_point;

    if (absolute_x_offset * tick_half_height > absolute_y_offset * tick_half_width) {
      label_point = GPoint(centre.x + (midpoint_x_offset < 0 ? -label_half_width : label_half_width),
                           tick_midpoint.y);
    } else {
      label_point = GPoint(tick_midpoint.x,
                           centre.y + (midpoint_y_offset < 0 ? -label_half_height : label_half_height));
    }

    const GPoint label_offset = prv_flint_label_offset(hour);
    label_point.x += label_offset.x;
    label_point.y += label_offset.y;
    prv_draw_flint_label(ctx, hour, label_point);
  }

  const int32_t hand_angle = time.minutes_today * TRIG_MAX_ANGLE / (HOURS_PER_DAY * 60);
  graphics_context_set_stroke_color(ctx, prv_copper());
  graphics_context_set_stroke_width(ctx, 4);
  graphics_draw_line(ctx, prv_point_on_circle(centre, 18, hand_angle + TRIG_MAX_ANGLE / 2),
                     prv_point_on_circle(centre, hand_radius, hand_angle));
  graphics_context_set_fill_color(ctx, prv_copper());
  graphics_fill_circle(ctx, centre, 7);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, centre, 7);
}
#endif

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
#elif defined(PBL_PLATFORM_FLINT) || defined(PBL_PLATFORM_DIORITE)
  prv_draw_flint_face(ctx, bounds, time);
#else
  prv_draw_dial(ctx, bounds, time, 5, 12);
#endif
}

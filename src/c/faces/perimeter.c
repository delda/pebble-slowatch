#include "perimeter.h"

#include "../settings.h"

#if defined(PBL_PLATFORM_GABBRO) || defined(PBL_PLATFORM_CHALK) || \
    defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_BASALT)
static int32_t prv_angle(int32_t angle) {
  if (settings_midnight_position() == MIDNIGHT_POSITION_BOTTOM) {
    angle += TRIG_MAX_ANGLE / 2;
  }
  return angle % TRIG_MAX_ANGLE;
}

static GPoint prv_point(GPoint centre, int16_t radius, int32_t angle) {
  return GPoint(centre.x + sin_lookup(angle) * radius / TRIG_MAX_RATIO,
                centre.y - cos_lookup(angle) * radius / TRIG_MAX_RATIO);
}

#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_BASALT)
// Intersect the hour's ray with an inset rectangle, as in the other rectangular dials.
static GPoint prv_rectangle_point(GPoint centre, int16_t half_width,
                                   int16_t half_height, int32_t angle) {
  const int32_t sine = sin_lookup(angle);
  const int32_t cosine = cos_lookup(angle);
  const int32_t abs_sine = sine < 0 ? -sine : sine;
  const int32_t abs_cosine = cosine < 0 ? -cosine : cosine;
  if (abs_sine * half_height > abs_cosine * half_width) {
    return GPoint(centre.x + sine * half_width / abs_sine,
                  centre.y - cosine * half_width / abs_sine);
  }
  return GPoint(centre.x + sine * half_height / abs_cosine,
                centre.y - cosine * half_height / abs_cosine);
}
#endif

void perimeter_face_draw(Layer *layer, GContext *ctx, ClockTime time) {
  const GRect bounds = layer_get_bounds(layer);
  const GPoint centre = grect_center_point(&bounds);
  const int16_t side = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
#if !defined(PBL_PLATFORM_EMERY) && !defined(PBL_PLATFORM_BASALT)
  const int16_t label_radius = side / 2 - 9;
#endif
  const int16_t tick_radius = side / 2 - 18;
#if defined(PBL_PLATFORM_CHALK) || defined(PBL_PLATFORM_BASALT)
  const int16_t tick_length = 5;
  const int16_t hour_stroke = 2;
  const int16_t hand_stroke = 3;
  const int16_t hand_tail = 12;
  const int16_t hub_radius = 5;
#else
  const int16_t tick_length = 7;
  const int16_t hour_stroke = 3;
  const int16_t hand_stroke = 4;
  const int16_t hand_tail = 18;
  const int16_t hub_radius = 7;
#endif
  const GColor detail = GColorFromHEX(settings_detail_color());
  const GColor hand = GColorFromHEX(settings_hand_color());

  graphics_context_set_fill_color(ctx, GColorFromHEX(settings_background_color()));
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_text_color(ctx, detail);
  const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  for (int hour = 0; hour < HOURS_PER_DAY; hour++) {
    char label[3];
    snprintf(label, sizeof(label), "%d", hour);
    const int32_t angle = prv_angle(hour * TRIG_MAX_ANGLE / HOURS_PER_DAY);
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_BASALT)
    const GPoint point = prv_rectangle_point(centre, bounds.size.w / 2 - 9,
                                              bounds.size.h / 2 - 9, angle);
#else
    const GPoint point = prv_point(centre, label_radius, angle);
#endif
    graphics_draw_text(ctx, label, font, GRect(point.x - 10, point.y - 8, 20, 16),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }

  graphics_context_set_stroke_color(ctx, detail);
  for (int tick = 0; tick < HOURS_PER_DAY * 4; tick++) {
    const int32_t angle = prv_angle(tick * TRIG_MAX_ANGLE / (HOURS_PER_DAY * 4));
    graphics_context_set_stroke_width(ctx, tick % 4 == 0 ? hour_stroke : 1);
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_BASALT)
#if defined(PBL_PLATFORM_BASALT)
    const int16_t tick_inset = 18;
#else
    const int16_t tick_inset = 20;
#endif
    const GPoint outer = prv_rectangle_point(centre, bounds.size.w / 2 - tick_inset,
                                              bounds.size.h / 2 - tick_inset, angle);
    graphics_draw_line(ctx, outer, prv_point(outer, -tick_length, angle));
#else
    graphics_draw_line(ctx, prv_point(centre, tick_radius, angle),
                       prv_point(centre, tick_radius - tick_length, angle));
#endif
  }

  const int32_t hand_angle = prv_angle(
      time.minutes_today * TRIG_MAX_ANGLE / (HOURS_PER_DAY * 60));
  graphics_context_set_stroke_color(ctx, hand);
  graphics_context_set_stroke_width(ctx, hand_stroke);
  graphics_draw_line(ctx, prv_point(centre, hand_tail, hand_angle + TRIG_MAX_ANGLE / 2),
                     prv_point(centre, tick_radius - tick_length - 2, hand_angle));
  graphics_context_set_fill_color(ctx, hand);
  graphics_fill_circle(ctx, centre, hub_radius);
  graphics_context_set_stroke_color(ctx, detail);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, centre, hub_radius);
}
#endif

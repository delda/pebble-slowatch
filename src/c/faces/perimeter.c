#include "perimeter.h"

#include "../settings.h"

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

#if !defined(PBL_ROUND)
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
#if defined(PBL_ROUND)
  const int16_t label_radius = side / 2 - 9;
#endif
  const int16_t tick_radius = side / 2 - 18;
#if !defined(PBL_PLATFORM_GABBRO) && !defined(PBL_PLATFORM_EMERY)
  const int16_t tick_length = 5;
  const int16_t hour_stroke = 2;
  const int16_t hand_tail = 12;
  const int16_t hub_radius = 5;
#else
  const int16_t tick_length = 7;
  const int16_t hour_stroke = 3;
  const int16_t hand_tail = 18;
  const int16_t hub_radius = 7;
#endif
  const GColor detail = GColorFromHEX(settings_detail_color());
  const GColor hand = PBL_IF_COLOR_ELSE(GColorFromHEX(settings_hand_color()), detail);

  graphics_context_set_fill_color(ctx, GColorFromHEX(settings_background_color()));
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_text_color(ctx, detail);
  const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  for (int hour = 0; hour < HOURS_PER_DAY; hour++) {
    char label[3];
    snprintf(label, sizeof(label), "%d", hour);
    const int32_t angle = prv_angle(hour * TRIG_MAX_ANGLE / HOURS_PER_DAY);
#if !defined(PBL_ROUND)
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
#if !defined(PBL_ROUND)
#if defined(PBL_PLATFORM_EMERY)
    const int16_t tick_inset = 20;
#else
    const int16_t tick_inset = 18;
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
  graphics_context_set_fill_color(ctx, hand);
  const int32_t tail_angle = hand_angle + TRIG_MAX_ANGLE / 2;
  const GPoint tail_end = prv_point(centre, hand_tail, tail_angle);
  const GPoint hand_tip = prv_point(centre, tick_radius - tick_length + 3, hand_angle);
  const int16_t base_half_width = 6;
  const int16_t tip_half_width = 1;
  // Round the tip offsets so a one-pixel half-width survives diagonal angles.
  const int32_t tip_dx = cos_lookup(hand_angle) * tip_half_width;
  const int32_t tip_dy = sin_lookup(hand_angle) * tip_half_width;
  const GPoint tip_offset = GPoint(
      (tip_dx + (tip_dx >= 0 ? TRIG_MAX_RATIO / 2 : -TRIG_MAX_RATIO / 2)) / TRIG_MAX_RATIO,
      (tip_dy + (tip_dy >= 0 ? TRIG_MAX_RATIO / 2 : -TRIG_MAX_RATIO / 2)) / TRIG_MAX_RATIO);
  GPoint hand_points[] = {
    prv_point(tail_end, base_half_width, tail_angle + TRIG_MAX_ANGLE / 4),
    GPoint(hand_tip.x - tip_offset.x, hand_tip.y - tip_offset.y),
    GPoint(hand_tip.x + tip_offset.x, hand_tip.y + tip_offset.y),
    prv_point(tail_end, base_half_width, tail_angle - TRIG_MAX_ANGLE / 4),
  };
  const GPathInfo hand_info = {.num_points = 4, .points = hand_points};
  GPath *hand_path = gpath_create(&hand_info);
  if (hand_path != NULL) {
    gpath_draw_filled(ctx, hand_path);
    gpath_destroy(hand_path);
  }
  graphics_fill_circle(ctx, centre, hub_radius);
  graphics_context_set_stroke_color(ctx, detail);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, centre, hub_radius);
}

#include "perimeter.h"

#include "../settings.h"
#include "dial_geometry.h"
#include "layout_metrics.h"

void perimeter_face_draw(Layer *layer, GContext *ctx, ClockTime time) {
  const PerimeterLayoutMetrics *layout = &perimeter_layout_metrics;
  const GRect bounds = layer_get_bounds(layer);
  const GPoint centre = grect_center_point(&bounds);
  const int16_t side = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
#if defined(PBL_ROUND)
  const int16_t label_radius = side / 2 - layout->label_inset;
#endif
  const int16_t tick_radius = side / 2 - layout->hand_inset;
  const GColor detail = GColorFromHEX(settings_detail_color());

  graphics_context_set_fill_color(ctx, GColorFromHEX(settings_background_color()));
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_text_color(ctx, detail);
  const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  for (int hour = 0; hour < HOURS_PER_DAY; hour++) {
    char label[3];
    snprintf(label, sizeof(label), "%d", hour);
    const int32_t angle = dial_geometry_angle(hour * TRIG_MAX_ANGLE / HOURS_PER_DAY);
#if !defined(PBL_ROUND)
    const GPoint point = dial_geometry_rectangle_point(centre, bounds.size.w / 2 - layout->label_inset,
                                              bounds.size.h / 2 - layout->label_inset, angle);
#else
    const GPoint point = dial_geometry_circle_point(centre, label_radius, angle);
#endif
    graphics_draw_text(ctx, label, font, GRect(point.x - 10, point.y - 8, 20, 16),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }

  graphics_context_set_stroke_color(ctx, detail);
  for (int tick = 0; tick < HOURS_PER_DAY * 4; tick++) {
    const int32_t angle = dial_geometry_angle(tick * TRIG_MAX_ANGLE / (HOURS_PER_DAY * 4));
    graphics_context_set_stroke_width(ctx, tick % 4 == 0 ? layout->hour_stroke : 1);
#if !defined(PBL_ROUND)
    const GPoint outer = dial_geometry_rectangle_point(centre, bounds.size.w / 2 - layout->tick_inset,
                                              bounds.size.h / 2 - layout->tick_inset, angle);
    graphics_draw_line(ctx, outer, dial_geometry_circle_point(outer, -layout->tick_length, angle));
#else
    graphics_draw_line(ctx, dial_geometry_circle_point(centre, tick_radius, angle),
                       dial_geometry_circle_point(centre, tick_radius - layout->tick_length, angle));
#endif
  }

  perimeter_draw_hand(ctx, bounds, time);
}

void perimeter_draw_hand(GContext *ctx, GRect bounds, ClockTime time) {
  const PerimeterLayoutMetrics *layout = &perimeter_layout_metrics;
  const GPoint centre = grect_center_point(&bounds);
  const int16_t side = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
  const int16_t tick_radius = side / 2 - layout->hand_inset;
  const GColor detail = GColorFromHEX(settings_detail_color());
  const GColor hand = PBL_IF_COLOR_ELSE(GColorFromHEX(settings_hand_color()), detail);
  graphics_context_set_antialiased(ctx, true);
  const int32_t hand_angle = dial_geometry_angle(
      time.minutes_today * TRIG_MAX_ANGLE / (HOURS_PER_DAY * 60));
  graphics_context_set_fill_color(ctx, hand);
  const int32_t tail_angle = hand_angle + TRIG_MAX_ANGLE / 2;
  const GPoint tail_end = dial_geometry_circle_point(centre, layout->hand_tail, tail_angle);
  const GPoint hand_tip = dial_geometry_circle_point(centre, tick_radius - layout->tick_length + 3, hand_angle);
  const int16_t base_half_width = 6;
  const int16_t tip_half_width = 1;
  // Round the tip offsets so a one-pixel half-width survives diagonal angles.
  const int32_t tip_dx = cos_lookup(hand_angle) * tip_half_width;
  const int32_t tip_dy = sin_lookup(hand_angle) * tip_half_width;
  const GPoint tip_offset = GPoint(
      (tip_dx + (tip_dx >= 0 ? TRIG_MAX_RATIO / 2 : -TRIG_MAX_RATIO / 2)) / TRIG_MAX_RATIO,
      (tip_dy + (tip_dy >= 0 ? TRIG_MAX_RATIO / 2 : -TRIG_MAX_RATIO / 2)) / TRIG_MAX_RATIO);
  GPoint hand_points[] = {
    dial_geometry_circle_point(tail_end, base_half_width, tail_angle + TRIG_MAX_ANGLE / 4),
    GPoint(hand_tip.x - tip_offset.x, hand_tip.y - tip_offset.y),
    GPoint(hand_tip.x + tip_offset.x, hand_tip.y + tip_offset.y),
    dial_geometry_circle_point(tail_end, base_half_width, tail_angle - TRIG_MAX_ANGLE / 4),
  };
  // The path and its points only need to live until this draw call returns.
  GPath hand_path = {
    .num_points = 4,
    .points = hand_points,
    .rotation = 0,
    .offset = GPoint(0, 0),
  };
  gpath_draw_filled(ctx, &hand_path);
  graphics_fill_circle(ctx, centre, layout->hub_radius);
  graphics_context_set_stroke_color(ctx, detail);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, centre, layout->hub_radius);
}

#include "classic_layout.h"

#include "../settings.h"
#include "dial_geometry.h"
#include "layout_metrics.h"

#define TICKS_PER_HOUR 4

static GFont s_label_font;

void classic_layout_init(void) {
  s_label_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_BOLD_14));
}

void classic_layout_deinit(void) {
  fonts_unload_custom_font(s_label_font);
  s_label_font = NULL;
}

static void prv_draw_hour_label(GContext *ctx, int hour, GPoint point, bool double_dial) {
  char label[3];
  snprintf(label, sizeof(label), "%d", hour % HOURS_PER_DAY);
  const GFont font = double_dial ? s_label_font : fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  graphics_draw_text(ctx, label, font,
                     GRect(point.x - 10, point.y - 8, 20, double_dial ? 18 : 16),
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static GColor prv_background_color(void) {
  return GColorFromHEX(settings_background_color());
}

static GColor prv_hand_color(void) {
  return GColorFromHEX(settings_hand_color());
}

static GColor prv_detail_color(void) {
  return GColorFromHEX(settings_detail_color());
}

#if defined(PBL_ROUND)
static void prv_draw_tick(GContext *ctx, GPoint centre, int16_t radius,
                          int32_t angle, int tick, bool split_long_ticks) {
  const bool half_hour_tick = (tick % TICKS_PER_HOUR) == TICKS_PER_HOUR / 2;
  const ClassicLayoutMetrics *layout = &classic_layout_metrics;
  const int16_t length = half_hour_tick && !split_long_ticks
      ? layout->long_tick_length : layout->short_tick_length;
  const int16_t stroke_width =
      (layout->bold_hour_ticks || split_long_ticks) && tick % TICKS_PER_HOUR == 0 ? 2 : 1;
  graphics_context_set_stroke_color(ctx, prv_detail_color());
  graphics_context_set_stroke_width(ctx, stroke_width);
  graphics_draw_line(ctx, dial_geometry_circle_point(centre, radius, angle),
                     dial_geometry_circle_point(centre, radius - length, angle));
  if (split_long_ticks && half_hour_tick) {
    const int16_t second_radius = radius - length - 3 - 9;
    graphics_draw_line(ctx, dial_geometry_circle_point(centre, second_radius, angle),
                       dial_geometry_circle_point(centre, second_radius - length, angle));
  }
}

static void prv_draw_dial(GContext *ctx, GRect bounds, ClockTime time,
                          bool split_long_ticks) {
  const ClassicLayoutMetrics *layout = &classic_layout_metrics;
  GPoint centre = grect_center_point(&bounds);
  const int16_t smallest_side = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
  const int16_t outer_radius = smallest_side / 2 - layout->edge_inset;
  const int16_t tick_radius = outer_radius - layout->tick_inset;
  const int16_t label_radius = tick_radius - layout->label_inset;

  graphics_context_set_fill_color(ctx, prv_background_color());
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_stroke_color(ctx, prv_detail_color());
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, centre, outer_radius);

  for (int tick = 0; tick < HOURS_PER_DAY * TICKS_PER_HOUR; tick++) {
    prv_draw_tick(ctx, centre, tick_radius,
                  dial_geometry_angle(tick * TRIG_MAX_ANGLE / (HOURS_PER_DAY * TICKS_PER_HOUR)),
                  tick, split_long_ticks);
  }

  graphics_context_set_text_color(ctx, prv_detail_color());
  for (int hour = 1; hour <= HOURS_PER_DAY; hour++) {
    const int32_t angle = dial_geometry_angle(hour * TRIG_MAX_ANGLE / HOURS_PER_DAY);
    GPoint point = dial_geometry_circle_point(centre, label_radius, angle);
    prv_draw_hour_label(ctx, hour, point, split_long_ticks);
  }

  const int32_t hand_angle =
      dial_geometry_angle(time.minutes_today * TRIG_MAX_ANGLE / (HOURS_PER_DAY * 60));
  graphics_context_set_stroke_color(ctx, prv_hand_color());
  graphics_context_set_stroke_width(ctx, 4);
  graphics_draw_line(ctx, dial_geometry_circle_point(centre, layout->hand_tail, hand_angle + TRIG_MAX_ANGLE / 2),
                     dial_geometry_circle_point(centre, tick_radius, hand_angle));
  graphics_context_set_fill_color(ctx, prv_hand_color());
  graphics_fill_circle(ctx, centre, layout->hub_radius);
  graphics_context_set_stroke_color(ctx, prv_detail_color());
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, centre, layout->hub_radius);
}
#endif

#if !defined(PBL_ROUND)
static GPoint prv_rectangular_label_offset(int hour) {
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
    const GPoint opposite_offset = prv_rectangular_label_offset(hour + 12);
    return GPoint(-opposite_offset.x, -opposite_offset.y);
  }

  return GPoint(0, 0);
}

static int prv_rectangular_label_position(int hour) {
  if (settings_midnight_position() == MIDNIGHT_POSITION_BOTTOM) {
    hour += HOURS_PER_DAY / 2;
    if (hour > HOURS_PER_DAY) {
      hour -= HOURS_PER_DAY;
    }
  }
  return hour;
}

static void prv_draw_rectangular_face(GContext *ctx, GRect bounds, ClockTime time,
                                bool split_long_ticks) {
  const ClassicLayoutMetrics *layout = &classic_layout_metrics;
  const int16_t smallest_side = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
  const int16_t hand_radius = smallest_side / 2 - layout->hand_inset;
  const GPoint centre = grect_center_point(&bounds);
  const int16_t label_half_width = centre.x - layout->label_inset;
  const int16_t label_half_height = centre.y - layout->label_inset;
  const int16_t tick_half_width = centre.x - layout->tick_inset;
  const int16_t tick_half_height = centre.y - layout->tick_inset;

  graphics_context_set_fill_color(ctx, prv_background_color());
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_stroke_color(ctx, prv_detail_color());
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_rect(ctx, GRect(layout->edge_inset, layout->edge_inset,
                                bounds.size.w - 2 * layout->edge_inset,
                                bounds.size.h - 2 * layout->edge_inset));
  graphics_context_set_text_color(ctx, prv_detail_color());

  for (int tick = 0; tick < HOURS_PER_DAY * TICKS_PER_HOUR; tick++) {
    const int32_t angle =
        dial_geometry_angle(tick * TRIG_MAX_ANGLE / (HOURS_PER_DAY * TICKS_PER_HOUR));
    const bool half_hour_tick = (tick % TICKS_PER_HOUR) == TICKS_PER_HOUR / 2;
    // Half-hour marks nearest the four corners, at 37.5 degrees and reflections.
    const bool corner_tick = tick == 10 || tick == 38 || tick == 58 || tick == 86;
    const int16_t short_tick_length = layout->short_tick_length;
    const int16_t tick_length = half_hour_tick
        ? (corner_tick ? layout->corner_tick_length : layout->long_tick_length)
        : short_tick_length;
    const int16_t stroke_width =
        (layout->bold_hour_ticks || split_long_ticks) && tick % TICKS_PER_HOUR == 0 ? 2 : 1;
    const int16_t segment_length = split_long_ticks ? short_tick_length : tick_length;
    const GPoint outer_point = dial_geometry_rectangle_point(centre, tick_half_width,
                                                       tick_half_height, angle);
    // In Double, every outer mark uses the same radial length from the border.
    const GPoint inner_point = split_long_ticks
        ? dial_geometry_circle_point(outer_point, -short_tick_length, angle)
        : dial_geometry_rectangle_point(centre, tick_half_width - segment_length,
                                 tick_half_height - segment_length, angle);
    graphics_context_set_stroke_width(ctx, stroke_width);
    graphics_draw_line(ctx, outer_point, inner_point);
    if (split_long_ticks && half_hour_tick) {
      // Calculate both endpoints on the same ray from the dial's centre.
      const int16_t second_inset = segment_length + 3;
      GPoint second_outer = dial_geometry_rectangle_point(
          centre, tick_half_width - second_inset, tick_half_height - second_inset, angle);
      GPoint second_inner = dial_geometry_rectangle_point(
          centre, tick_half_width - second_inset - segment_length,
          tick_half_height - second_inset - segment_length, angle);
      // Move only the second segment 9 px inward along its ray.
      const GPoint offset = dial_geometry_circle_point(GPoint(0, 0), 9, angle);
      second_outer.x -= offset.x;
      second_outer.y -= offset.y;
      second_inner.x -= offset.x;
      second_inner.y -= offset.y;
      graphics_draw_line(ctx, second_outer, second_inner);
    }
  }

  for (int hour = 1; hour <= HOURS_PER_DAY; hour++) {
    const int32_t previous_tick_angle = dial_geometry_angle(
        (hour * TICKS_PER_HOUR - TICKS_PER_HOUR / 2) * TRIG_MAX_ANGLE /
        (HOURS_PER_DAY * TICKS_PER_HOUR));
    int32_t next_tick_angle =
        (hour * TICKS_PER_HOUR + TICKS_PER_HOUR / 2) * TRIG_MAX_ANGLE /
        (HOURS_PER_DAY * TICKS_PER_HOUR);
    next_tick_angle = dial_geometry_angle(next_tick_angle);
    const GPoint previous_tick = dial_geometry_rectangle_point(centre, tick_half_width,
                                                         tick_half_height,
                                                         previous_tick_angle);
    const GPoint next_tick = dial_geometry_rectangle_point(centre, tick_half_width,
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

    const GPoint label_offset = prv_rectangular_label_offset(prv_rectangular_label_position(hour));
    label_point.x += label_offset.x;
    label_point.y += label_offset.y;
    prv_draw_hour_label(ctx, hour, label_point, split_long_ticks);
  }

  const int32_t hand_angle =
      dial_geometry_angle(time.minutes_today * TRIG_MAX_ANGLE / (HOURS_PER_DAY * 60));
  graphics_context_set_stroke_color(ctx, prv_hand_color());
  graphics_context_set_stroke_width(ctx, 4);
  graphics_draw_line(ctx, dial_geometry_circle_point(centre, layout->hand_tail, hand_angle + TRIG_MAX_ANGLE / 2),
                     dial_geometry_circle_point(centre, hand_radius, hand_angle));
  graphics_context_set_fill_color(ctx, prv_hand_color());
  graphics_fill_circle(ctx, centre, layout->hub_radius);
  graphics_context_set_stroke_color(ctx, prv_detail_color());
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, centre, layout->hub_radius);
}
#endif

GColor classic_layout_background_color(void) {
  return prv_background_color();
}

void classic_layout_draw(Layer *layer, GContext *ctx, ClockTime time, bool split_long_ticks) {
  const GRect bounds = layer_get_bounds(layer);

#if defined(PBL_ROUND)
  prv_draw_dial(ctx, bounds, time, split_long_ticks);
#else
  prv_draw_rectangular_face(ctx, bounds, time, split_long_ticks);
#endif
}

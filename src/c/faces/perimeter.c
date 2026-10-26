#include "perimeter.h"

#include "../settings.h"

#if defined(PBL_PLATFORM_GABBRO)
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

void perimeter_face_draw(Layer *layer, GContext *ctx, ClockTime time) {
  const GRect bounds = layer_get_bounds(layer);
  const GPoint centre = grect_center_point(&bounds);
  const int16_t side = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
  const int16_t label_radius = side / 2 - 9;
  const int16_t tick_radius = side / 2 - 18;
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
    const GPoint point = prv_point(centre, label_radius,
                                   prv_angle(hour * TRIG_MAX_ANGLE / HOURS_PER_DAY));
    graphics_draw_text(ctx, label, font, GRect(point.x - 10, point.y - 8, 20, 16),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }

  graphics_context_set_stroke_color(ctx, detail);
  for (int tick = 0; tick < HOURS_PER_DAY * 4; tick++) {
    const int32_t angle = prv_angle(tick * TRIG_MAX_ANGLE / (HOURS_PER_DAY * 4));
    graphics_context_set_stroke_width(ctx, tick % 4 == 0 ? 3 : 1);
    graphics_draw_line(ctx, prv_point(centre, tick_radius, angle),
                       prv_point(centre, tick_radius - 7, angle));
  }

  const int32_t hand_angle = prv_angle(
      time.minutes_today * TRIG_MAX_ANGLE / (HOURS_PER_DAY * 60));
  graphics_context_set_stroke_color(ctx, hand);
  graphics_context_set_stroke_width(ctx, 4);
  graphics_draw_line(ctx, prv_point(centre, 18, hand_angle + TRIG_MAX_ANGLE / 2),
                     prv_point(centre, tick_radius - 9, hand_angle));
  graphics_context_set_fill_color(ctx, hand);
  graphics_fill_circle(ctx, centre, 7);
  graphics_context_set_stroke_color(ctx, detail);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, centre, 7);
}
#endif

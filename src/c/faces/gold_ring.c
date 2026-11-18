#include "gold_ring.h"

#if defined(PBL_PLATFORM_GABBRO) || defined(PBL_PLATFORM_CHALK)
#include "dial_geometry.h"

#if defined(PBL_PLATFORM_CHALK)
enum { LABEL_WIDTH = 16, LABEL_HEIGHT = 20, LABEL_INSET = 24,
       HOUR_TICK_LENGTH = 12, HALF_HOUR_TICK_LENGTH = 8, SHORT_TICK_LENGTH = 5,
       RING_SIZE = 8, RING_OFFSET = 2, HAND_STROKE = 2, HUB_SIZE = 5 };
#else
enum { LABEL_WIDTH = 22, LABEL_HEIGHT = 28, LABEL_INSET = 34,
       HOUR_TICK_LENGTH = 17, HALF_HOUR_TICK_LENGTH = 11, SHORT_TICK_LENGTH = 7,
       RING_SIZE = 11, RING_OFFSET = 3, HAND_STROKE = 3, HUB_SIZE = 7 };
#endif

GColor gold_ring_face_background_color(void) {
  return GColorFromHEX(0xFFFFAA);
}

// Render the system font in a temporary central box, then rotate that bitmap.
// The SDK's text API cannot rotate glyphs directly.
static void prv_draw_rotated_label(Layer *layer, GContext *ctx, GBitmap *label_bitmap,
                                   const char *label, GFont font, GRect scratch,
                                   GPoint point, int32_t rotation) {
  const GColor gold = gold_ring_face_background_color();
  graphics_context_set_fill_color(ctx, gold);
  graphics_fill_rect(ctx, scratch, 0, GCornerNone);
  graphics_draw_text(ctx, label, font, scratch,
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);

  GBitmap *frame = graphics_capture_frame_buffer(ctx);
  if (frame != NULL) {
    const GPoint origin = layer_convert_point_to_screen(layer, scratch.origin);
    const GRect frame_bounds = gbitmap_get_bounds(frame);
    for (int y = 0; y < scratch.size.h; y++) {
      const GBitmapDataRowInfo dest = gbitmap_get_data_row_info(label_bitmap, y);
      const int screen_y = origin.y + y;
      const bool visible_row = screen_y >= frame_bounds.origin.y &&
          screen_y < frame_bounds.origin.y + frame_bounds.size.h;
      const GBitmapDataRowInfo source = visible_row
          ? gbitmap_get_data_row_info(frame, screen_y) : (GBitmapDataRowInfo){0};
      for (int x = 0; x < scratch.size.w; x++) {
        const int screen_x = origin.x + x;
        const uint8_t pixel = visible_row && screen_x >= source.min_x && screen_x <= source.max_x
            ? source.data[screen_x] : gold.argb;
        // Transparent padding prevents a rotated box from covering adjacent labels.
        dest.data[x] = pixel == gold.argb ? GColorClear.argb : pixel;
      }
    }
    graphics_release_frame_buffer(ctx, frame);
  }
  graphics_fill_rect(ctx, scratch, 0, GCornerNone);
  if (frame != NULL) {
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_rotated_bitmap(ctx, label_bitmap,
                                GPoint(scratch.size.w / 2, scratch.size.h / 2), rotation, point);
    graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  } else {
    graphics_draw_text(ctx, label, font,
                       GRect(point.x - LABEL_WIDTH / 2, point.y - LABEL_HEIGHT / 2,
                             LABEL_WIDTH, LABEL_HEIGHT),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
}

void gold_ring_face_draw(Layer *layer, GContext *ctx, ClockTime time) {
  const GRect bounds = layer_get_bounds(layer);
  const GPoint centre = grect_center_point(&bounds);
  const int16_t side = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
  const int16_t tick_radius = side / 2 - 1;
  const int16_t label_radius = side / 2 - LABEL_INSET;
  const GColor gold = gold_ring_face_background_color();

  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, gold);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_stroke_color(ctx, GColorBlack);

  // Six subdivisions per hour: each small mark represents ten minutes.
  for (int tick = 0; tick < 144; tick++) {
    const int32_t angle = dial_geometry_angle(tick * TRIG_MAX_ANGLE / 144);
    const bool full_hour = tick % 6 == 0;
    const int16_t length = full_hour ? HOUR_TICK_LENGTH
        : (tick % 3 == 0 ? HALF_HOUR_TICK_LENGTH : SHORT_TICK_LENGTH);
    graphics_context_set_stroke_width(ctx, full_hour ? 2 : 1);
    graphics_draw_line(ctx, dial_geometry_circle_point(centre, tick_radius, angle),
                       dial_geometry_circle_point(centre, tick_radius - length, angle));
  }

  graphics_context_set_text_color(ctx, GColorBlack);
#if defined(PBL_PLATFORM_CHALK)
  const GFont custom_font = fonts_load_custom_font(
      resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_REGULAR_14));
  const GFont label_font = custom_font != NULL ? custom_font : fonts_get_system_font(FONT_KEY_GOTHIC_14);
#else
  const GFont label_font = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
#endif
  GBitmap *label_bitmap = gbitmap_create_blank(GSize(LABEL_WIDTH, LABEL_HEIGHT), GBitmapFormat8Bit);
  const GRect scratch = GRect(centre.x - LABEL_WIDTH / 2, centre.y - LABEL_HEIGHT / 2,
                             LABEL_WIDTH, LABEL_HEIGHT);
  for (int hour = 0; hour < 24; hour++) {
    const int32_t angle = dial_geometry_angle(hour * TRIG_MAX_ANGLE / 24);
    const GPoint point = dial_geometry_circle_point(centre, label_radius + 2, angle);
    char label[3];
    snprintf(label, sizeof(label), "%d", hour);
    if (label_bitmap != NULL) {
      // Keep the requested orientation tied to the hour: 0=180°, 6=270°, 18=90°.
      const int32_t rotation = (hour * TRIG_MAX_ANGLE / 24 + TRIG_MAX_ANGLE / 2) % TRIG_MAX_ANGLE;
      prv_draw_rotated_label(layer, ctx, label_bitmap, label, label_font, scratch, point, rotation);
    } else {
      graphics_draw_text(ctx, label, label_font,
                         GRect(point.x - LABEL_WIDTH / 2, point.y - LABEL_HEIGHT / 2,
                               LABEL_WIDTH, LABEL_HEIGHT),
                         GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    }
  }
  gbitmap_destroy(label_bitmap);
#if defined(PBL_PLATFORM_CHALK)
  if (custom_font != NULL) {
    fonts_unload_custom_font(custom_font);
  }
#endif

  const int32_t angle = dial_geometry_angle(time.minutes_today * TRIG_MAX_ANGLE / (24 * 60));
  const GPoint tip = dial_geometry_circle_point(centre, tick_radius, angle);
  const int16_t ring_radius = label_radius + RING_OFFSET;
  const GPoint ring = dial_geometry_circle_point(centre, ring_radius, angle);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, HAND_STROKE);
  graphics_draw_line(ctx, centre,
                     dial_geometry_circle_point(centre, ring_radius - RING_SIZE - 1, angle));
  graphics_draw_line(ctx, dial_geometry_circle_point(centre, ring_radius + RING_SIZE + 1, angle), tip);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, ring, RING_SIZE);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, centre, HUB_SIZE);
}
#endif

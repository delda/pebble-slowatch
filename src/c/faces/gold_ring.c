#include "gold_ring.h"

#include "dial_geometry.h"
#include "../settings.h"

#if defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_APLITE) || defined(PBL_PLATFORM_DIORITE) || defined(PBL_PLATFORM_FLINT)
enum { LABEL_WIDTH = 18, LABEL_HEIGHT = 22, LABEL_INSET = 24,
       HOUR_TICK_LENGTH = 12, HALF_HOUR_TICK_LENGTH = 8, SHORT_TICK_LENGTH = 5,
       RING_SIZE = 8, RING_OFFSET = 2, HAND_STROKE = 2, HUB_SIZE = 5 };
#elif defined(PBL_PLATFORM_CHALK)
enum { LABEL_WIDTH = 16, LABEL_HEIGHT = 20, LABEL_INSET = 24,
       HOUR_TICK_LENGTH = 12, HALF_HOUR_TICK_LENGTH = 8, SHORT_TICK_LENGTH = 5,
       RING_SIZE = 8, RING_OFFSET = 2, HAND_STROKE = 2, HUB_SIZE = 5 };
#else
enum { LABEL_WIDTH = 22, LABEL_HEIGHT = 28, LABEL_INSET = 34,
       HOUR_TICK_LENGTH = 17, HALF_HOUR_TICK_LENGTH = 11, SHORT_TICK_LENGTH = 7,
       RING_SIZE = 11, RING_OFFSET = 3, HAND_STROKE = 3, HUB_SIZE = 7 };
#endif

GColor gold_ring_face_background_color(void) {
  return PBL_IF_COLOR_ELSE(GColorFromHEX(settings_background_color()), GColorWhite);
}

static GPoint prv_dial_point(GRect bounds, int16_t inset, int32_t angle) {
  const GPoint centre = grect_center_point(&bounds);
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_APLITE) || defined(PBL_PLATFORM_DIORITE) || defined(PBL_PLATFORM_FLINT)
  return dial_geometry_rectangle_point(centre, bounds.size.w / 2 - inset,
                                       bounds.size.h / 2 - inset, angle);
#else
  const int16_t side = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
  return dial_geometry_circle_point(centre, side / 2 - inset, angle);
#endif
}

// Render the system font in a temporary central box, then rotate that bitmap.
// The SDK's text API cannot rotate glyphs directly.
static void prv_draw_rotated_label(Layer *layer, GContext *ctx, GBitmap *label_bitmap,
                                   const char *label, GFont font, GRect scratch,
                                   GPoint point, int32_t rotation) {
  const GColor background = gold_ring_face_background_color();
  graphics_context_set_fill_color(ctx, background);
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
        const bool visible_pixel = visible_row && screen_x >= source.min_x && screen_x <= source.max_x;
#if defined(PBL_COLOR)
        const uint8_t pixel = visible_pixel
            ? source.data[screen_x] : background.argb;
        // Transparent padding prevents a rotated box from covering adjacent labels.
        dest.data[x] = pixel == background.argb ? GColorClear.argb : pixel;
#else
        const bool white = !visible_pixel || (source.data[screen_x / 8] & (1 << (screen_x % 8)));
        const uint8_t mask = 1 << (x % 8);
        if (white) {
          dest.data[x / 8] |= mask;
        } else {
          dest.data[x / 8] &= ~mask;
        }
#endif
      }
    }
    graphics_release_frame_buffer(ctx, frame);
  }
  graphics_fill_rect(ctx, scratch, 0, GCornerNone);
  if (frame != NULL) {
    graphics_context_set_compositing_mode(ctx, PBL_IF_COLOR_ELSE(GCompOpSet, GCompOpAnd));
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
  const GColor background = gold_ring_face_background_color();
  const GColor detail = PBL_IF_COLOR_ELSE(GColorFromHEX(settings_detail_color()), GColorBlack);
  const GColor hand = PBL_IF_COLOR_ELSE(GColorFromHEX(settings_hand_color()), GColorBlack);

  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, background);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_stroke_color(ctx, detail);

  // Six subdivisions per hour: each small mark represents ten minutes.
  for (int tick = 0; tick < 144; tick++) {
    const int32_t angle = dial_geometry_angle(tick * TRIG_MAX_ANGLE / 144);
    const bool full_hour = tick % 6 == 0;
    const int16_t length = full_hour ? HOUR_TICK_LENGTH
        : (tick % 3 == 0 ? HALF_HOUR_TICK_LENGTH : SHORT_TICK_LENGTH);
    graphics_context_set_stroke_width(ctx, full_hour ? 2 : 1);
    const GPoint outer = prv_dial_point(bounds, 1, angle);
    graphics_draw_line(ctx, outer, dial_geometry_circle_point(outer, -length, angle));
  }

  graphics_context_set_text_color(ctx, detail);
#if defined(PBL_PLATFORM_CHALK) || defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_APLITE) || defined(PBL_PLATFORM_DIORITE) || defined(PBL_PLATFORM_FLINT)
#if defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_APLITE) || defined(PBL_PLATFORM_DIORITE) || defined(PBL_PLATFORM_FLINT)
  const uint32_t font_resource = RESOURCE_ID_FONT_ROBOTO_CONDENSED_REGULAR_16;
#else
  const uint32_t font_resource = RESOURCE_ID_FONT_ROBOTO_CONDENSED_REGULAR_14;
#endif
  const GFont custom_font = fonts_load_custom_font(
      resource_get_handle(font_resource));
  const GFont label_font = custom_font != NULL ? custom_font : fonts_get_system_font(FONT_KEY_GOTHIC_14);
#else
  const GFont label_font = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
#endif
  GBitmap *label_bitmap = gbitmap_create_blank(GSize(LABEL_WIDTH, LABEL_HEIGHT),
      PBL_IF_COLOR_ELSE(GBitmapFormat8Bit, GBitmapFormat1Bit));
  const GRect scratch = GRect(centre.x - LABEL_WIDTH / 2, centre.y - LABEL_HEIGHT / 2,
                             LABEL_WIDTH, LABEL_HEIGHT);
  for (int hour = 0; hour < 24; hour++) {
    const int32_t angle = dial_geometry_angle(hour * TRIG_MAX_ANGLE / 24);
    const GPoint point = prv_dial_point(bounds, LABEL_INSET - 2, angle);
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
#if defined(PBL_PLATFORM_CHALK) || defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_APLITE) || defined(PBL_PLATFORM_DIORITE) || defined(PBL_PLATFORM_FLINT)
  if (custom_font != NULL) {
    fonts_unload_custom_font(custom_font);
  }
#endif

  const int32_t angle = dial_geometry_angle(time.minutes_today * TRIG_MAX_ANGLE / (24 * 60));
  const GPoint tip = prv_dial_point(bounds, 1, angle);
  const GPoint ring = prv_dial_point(bounds, LABEL_INSET - RING_OFFSET, angle);
  graphics_context_set_stroke_color(ctx, hand);
  graphics_context_set_stroke_width(ctx, HAND_STROKE);
  graphics_draw_line(ctx, centre,
                     dial_geometry_circle_point(ring, -RING_SIZE - 1, angle));
  graphics_draw_line(ctx, dial_geometry_circle_point(ring, RING_SIZE + 1, angle), tip);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, ring, RING_SIZE);
  graphics_context_set_fill_color(ctx, hand);
  graphics_fill_circle(ctx, centre, HUB_SIZE);
}

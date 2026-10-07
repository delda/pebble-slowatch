#include "settings.h"

enum {
  PERSIST_KEY_MIDNIGHT_POSITION = 1,
  PERSIST_KEY_BACKGROUND_COLOR = 2,
  PERSIST_KEY_HAND_COLOR = 3,
  PERSIST_KEY_DETAIL_COLOR = 4,
  PERSIST_KEY_COLOR_LAYOUT = 5,
};

static MidnightPosition s_midnight_position;
static uint32_t s_background_color;
static uint32_t s_hand_color;
static uint32_t s_detail_color;
static ColorLayout s_color_layout;
static SettingsChangedHandler s_changed_handler;

static void prv_notify_changed(void) {
  if (s_changed_handler != NULL) {
    s_changed_handler();
  }
}

void settings_set_midnight_position(MidnightPosition position) {
  if (position == s_midnight_position) {
    return;
  }

  s_midnight_position = position;
  persist_write_int(PERSIST_KEY_MIDNIGHT_POSITION, position);
  prv_notify_changed();
}

static void prv_set_color(uint32_t *current, uint32_t color, uint32_t persist_key) {
  if (*current == color) {
    return;
  }

  *current = color;
  persist_write_int(persist_key, (int)color);
  prv_notify_changed();
}

static void prv_apply_color_layout(ColorLayout layout) {
  switch (layout) {
    // Keep the original SloWatch palette as the first/default layout.
    case COLOR_LAYOUT_DEFAULT:
      s_background_color = 0x062D54; // Navy
      s_hand_color = 0xC98858;       // Copper
      s_detail_color = 0xFFFFFF;     // White
      break;
    case COLOR_LAYOUT_SNOW:          // White / Black / Gray
      s_background_color = 0xFFFFFF;
      s_hand_color = 0x000000;
      s_detail_color = 0xAAAAAA;
      break;
    case COLOR_LAYOUT_NIGHT:         // Black / White / Gray
      s_background_color = 0x000000;
      s_hand_color = 0xFFFFFF;
      s_detail_color = 0xAAAAAA;
      break;
    case COLOR_LAYOUT_SAND:          // Beige / Black / Gray
      s_background_color = 0xAA5500; // Pebble Windsor Tan
      s_hand_color = 0x000000;
      s_detail_color = 0xAAAAAA;
      break;
    case COLOR_LAYOUT_SKY:           // Blue / White / Gray
      s_background_color = 0x0000FF;
      s_hand_color = 0xFFFFFF;
      s_detail_color = 0xAAAAAA;
      break;
    case COLOR_LAYOUT_VANILLA:       // White / Black / Beige
      s_background_color = 0xFFFFFF;
      s_hand_color = 0x000000;
      s_detail_color = 0xAA5500;
      break;
    case COLOR_LAYOUT_SUN:           // Yellow / Black / Yellow
      s_background_color = 0xFFFF00;
      s_hand_color = 0x000000;
      s_detail_color = 0xFFFF00;
      break;
    case COLOR_LAYOUT_MEADOW:        // White / Black / Green
      s_background_color = 0xFFFFFF;
      s_hand_color = 0x000000;
      s_detail_color = 0x00AA00;
      break;
    case COLOR_LAYOUT_MINT:          // Light green / Black / Gray
      s_background_color = 0x55FF55; // Pebble Screamin Green
      s_hand_color = 0x000000;
      s_detail_color = 0xAAAAAA;
      break;
    case COLOR_LAYOUT_FOREST:        // Dark green / White / Yellow
      s_background_color = 0x005500;
      s_hand_color = 0xFFFFFF;
      s_detail_color = 0xFFFF00;
      break;
    default:
      layout = COLOR_LAYOUT_DEFAULT;
      prv_apply_color_layout(layout);
      return;
  }

  s_color_layout = layout;
  persist_write_int(PERSIST_KEY_COLOR_LAYOUT, layout);
  persist_write_int(PERSIST_KEY_BACKGROUND_COLOR, (int)s_background_color);
  persist_write_int(PERSIST_KEY_HAND_COLOR, (int)s_hand_color);
  persist_write_int(PERSIST_KEY_DETAIL_COLOR, (int)s_detail_color);
  prv_notify_changed();
}

void settings_init(SettingsChangedHandler changed_handler) {
  s_changed_handler = changed_handler;
  s_midnight_position = MIDNIGHT_POSITION_BOTTOM;
  s_background_color = 0x062D54;
  s_hand_color = 0xC98858;
  s_detail_color = 0xFFFFFF;
  s_color_layout = COLOR_LAYOUT_DEFAULT;
  if (persist_exists(PERSIST_KEY_MIDNIGHT_POSITION)) {
    const int stored_position = persist_read_int(PERSIST_KEY_MIDNIGHT_POSITION);
    if (stored_position == MIDNIGHT_POSITION_BOTTOM) {
      s_midnight_position = MIDNIGHT_POSITION_BOTTOM;
    }
  }

  if (persist_exists(PERSIST_KEY_BACKGROUND_COLOR)) {
    s_background_color = (uint32_t)persist_read_int(PERSIST_KEY_BACKGROUND_COLOR);
  }
  if (persist_exists(PERSIST_KEY_HAND_COLOR)) {
    s_hand_color = (uint32_t)persist_read_int(PERSIST_KEY_HAND_COLOR);
  }
  if (persist_exists(PERSIST_KEY_DETAIL_COLOR)) {
    s_detail_color = (uint32_t)persist_read_int(PERSIST_KEY_DETAIL_COLOR);
  }

  if (persist_exists(PERSIST_KEY_COLOR_LAYOUT)) {
    const int stored_layout = persist_read_int(PERSIST_KEY_COLOR_LAYOUT);
    if (stored_layout >= COLOR_LAYOUT_DEFAULT && stored_layout <= COLOR_LAYOUT_FOREST) {
      s_color_layout = (ColorLayout)stored_layout;
    }
  }

}

void settings_deinit(void) {
  s_changed_handler = NULL;
}

MidnightPosition settings_midnight_position(void) {
  return s_midnight_position;
}

uint32_t settings_background_color(void) {
  return s_background_color;
}

void settings_set_background_color(uint32_t color) {
  prv_set_color(&s_background_color, color, PERSIST_KEY_BACKGROUND_COLOR);
}

uint32_t settings_hand_color(void) {
  return s_hand_color;
}

void settings_set_hand_color(uint32_t color) {
  prv_set_color(&s_hand_color, color, PERSIST_KEY_HAND_COLOR);
}

uint32_t settings_detail_color(void) {
  return s_detail_color;
}

void settings_set_detail_color(uint32_t color) {
  prv_set_color(&s_detail_color, color, PERSIST_KEY_DETAIL_COLOR);
}

void settings_set_color_layout(ColorLayout layout) {
  if (layout > COLOR_LAYOUT_FOREST) {
    layout = COLOR_LAYOUT_DEFAULT;
  }

  if (layout == s_color_layout) {
    return;
  }

  prv_apply_color_layout(layout);
}

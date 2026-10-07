#pragma once

#include <pebble.h>

typedef enum {
  MIDNIGHT_POSITION_TOP = 0,
  MIDNIGHT_POSITION_BOTTOM = 1,
} MidnightPosition;

typedef enum {
  COLOR_LAYOUT_DEFAULT = 0,
  COLOR_LAYOUT_SNOW = 1,
  COLOR_LAYOUT_NIGHT = 2,
  COLOR_LAYOUT_SAND = 3,
  COLOR_LAYOUT_SKY = 4,
  COLOR_LAYOUT_VANILLA = 5,
  COLOR_LAYOUT_SUN = 6,
  COLOR_LAYOUT_MEADOW = 7,
  COLOR_LAYOUT_MINT = 8,
  COLOR_LAYOUT_FOREST = 9,
} ColorLayout;

typedef void (*SettingsChangedHandler)(void);

void settings_init(SettingsChangedHandler changed_handler);
void settings_deinit(void);

MidnightPosition settings_midnight_position(void);
void settings_set_midnight_position(MidnightPosition position);

uint32_t settings_background_color(void);
void settings_set_background_color(uint32_t color);

uint32_t settings_hand_color(void);
void settings_set_hand_color(uint32_t color);

uint32_t settings_detail_color(void);
void settings_set_detail_color(uint32_t color);

void settings_set_color_layout(ColorLayout layout);

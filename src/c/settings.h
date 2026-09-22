#pragma once

#include <pebble.h>

typedef enum {
  MIDNIGHT_POSITION_TOP = 0,
  MIDNIGHT_POSITION_BOTTOM = 1,
} MidnightPosition;

typedef void (*SettingsChangedHandler)(void);

void settings_init(SettingsChangedHandler changed_handler);
void settings_deinit(void);

MidnightPosition settings_midnight_position(void);
void settings_set_midnight_position(MidnightPosition position);

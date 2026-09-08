#pragma once

#include <pebble.h>

typedef struct {
  int32_t minutes_today;
} ClockTime;

typedef void (*ClockUpdateHandler)(void);

void clock_init(ClockUpdateHandler update_handler);
void clock_deinit(void);
ClockTime clock_current_time(void);

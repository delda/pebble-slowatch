#include "clock.h"

static ClockUpdateHandler s_update_handler;

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (s_update_handler != NULL) {
    s_update_handler();
  }
}

void clock_init(ClockUpdateHandler update_handler) {
  s_update_handler = update_handler;
  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
}

void clock_deinit(void) {
  tick_timer_service_unsubscribe();
  s_update_handler = NULL;
}

ClockTime clock_current_time(void) {
  time_t now = time(NULL);
  struct tm *time_info = localtime(&now);

  return (ClockTime) {
    .minutes_today = time_info->tm_hour * 60 + time_info->tm_min,
  };
}

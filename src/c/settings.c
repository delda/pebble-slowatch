#include "settings.h"

enum {
  PERSIST_KEY_MIDNIGHT_POSITION = 1,
};

static MidnightPosition s_midnight_position;
static SettingsChangedHandler s_changed_handler;

void settings_set_midnight_position(MidnightPosition position) {
  if (position == s_midnight_position) {
    return;
  }

  s_midnight_position = position;
  persist_write_int(PERSIST_KEY_MIDNIGHT_POSITION, position);
  if (s_changed_handler != NULL) {
    s_changed_handler();
  }
}

void settings_init(SettingsChangedHandler changed_handler) {
  s_changed_handler = changed_handler;
#if defined(PBL_PLATFORM_FLINT)
  s_midnight_position = MIDNIGHT_POSITION_BOTTOM;
#else
  s_midnight_position = MIDNIGHT_POSITION_TOP;
#endif
  if (persist_exists(PERSIST_KEY_MIDNIGHT_POSITION)) {
    const int stored_position = persist_read_int(PERSIST_KEY_MIDNIGHT_POSITION);
    if (stored_position == MIDNIGHT_POSITION_BOTTOM) {
      s_midnight_position = MIDNIGHT_POSITION_BOTTOM;
    }
  }

}

void settings_deinit(void) {
  s_changed_handler = NULL;
}

MidnightPosition settings_midnight_position(void) {
  return s_midnight_position;
}

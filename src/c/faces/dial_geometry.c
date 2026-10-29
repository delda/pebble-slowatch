#include "dial_geometry.h"

#include "../settings.h"

int32_t dial_geometry_angle(int32_t angle) {
  if (settings_midnight_position() == MIDNIGHT_POSITION_BOTTOM) {
    angle += TRIG_MAX_ANGLE / 2;
  }
  return angle % TRIG_MAX_ANGLE;
}

GPoint dial_geometry_circle_point(GPoint centre, int16_t radius, int32_t angle) {
  return GPoint(centre.x + sin_lookup(angle) * radius / TRIG_MAX_RATIO,
                centre.y - cos_lookup(angle) * radius / TRIG_MAX_RATIO);
}

GPoint dial_geometry_rectangle_point(GPoint centre, int16_t half_width,
                                     int16_t half_height, int32_t angle) {
  const int32_t sine = sin_lookup(angle);
  const int32_t cosine = cos_lookup(angle);
  const int32_t absolute_sine = sine < 0 ? -sine : sine;
  const int32_t absolute_cosine = cosine < 0 ? -cosine : cosine;

  if (absolute_sine * half_height > absolute_cosine * half_width) {
    return GPoint(centre.x + sine * half_width / absolute_sine,
                  centre.y - cosine * half_width / absolute_sine);
  }

  return GPoint(centre.x + sine * half_height / absolute_cosine,
                centre.y - cosine * half_height / absolute_cosine);
}

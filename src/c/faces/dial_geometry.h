#pragma once

#include <pebble.h>

// Rotate an angle to the configured midnight position and wrap to one turn.
int32_t dial_geometry_angle(int32_t angle);
GPoint dial_geometry_circle_point(GPoint centre, int16_t radius, int32_t angle);
// Intersect a ray with a rectangle centred on centre. Half-sizes must be positive.
GPoint dial_geometry_rectangle_point(GPoint centre, int16_t half_width,
                                     int16_t half_height, int32_t angle);

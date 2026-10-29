#pragma once

#include <pebble.h>

// Insets and lengths are in pixels. Classic and Double share these metrics.
typedef struct {
  int16_t edge_inset;
  int16_t label_inset;
  int16_t tick_inset;
  int16_t short_tick_length;
  int16_t long_tick_length;
  int16_t corner_tick_length;
  bool bold_hour_ticks;
  int16_t hand_inset;
  int16_t hand_tail;
  int16_t hub_radius;
} ClassicLayoutMetrics;

typedef struct {
  int16_t label_inset;
  int16_t tick_inset;
  int16_t hand_inset;
  int16_t tick_length;
  int16_t hour_stroke;
  int16_t hand_tail;
  int16_t hub_radius;
} PerimeterLayoutMetrics;

extern const ClassicLayoutMetrics classic_layout_metrics;
extern const PerimeterLayoutMetrics perimeter_layout_metrics;

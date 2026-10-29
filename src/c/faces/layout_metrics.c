#include "layout_metrics.h"

const ClassicLayoutMetrics classic_layout_metrics = {
#if defined(PBL_ROUND)
#if defined(PBL_PLATFORM_GABBRO)
  .edge_inset = 8,
  .label_inset = 15,
#else
  .edge_inset = 5,
  .label_inset = 12,
#endif
  .tick_inset = 5,
  .short_tick_length = 4,
  .long_tick_length = 20,
  .corner_tick_length = 20,
  .bold_hour_ticks = true,
  .hand_inset = 0,
  .hand_tail = 20,
  .hub_radius = 8,
#else
  .edge_inset = 1,
#if defined(PBL_PLATFORM_FLINT)
  .label_inset = 13,
#else
  .label_inset = 15,
#endif
  .tick_inset = 2,
#if defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_FLINT)
  // Emery lengths scaled by 144 / 200, rounded to whole pixels.
  .short_tick_length = 3,
  .long_tick_length = 14,
  .corner_tick_length = 17,
  .bold_hour_ticks = true,
#elif defined(PBL_PLATFORM_EMERY)
  .short_tick_length = 4,
  .long_tick_length = 20,
  .corner_tick_length = 24,
  .bold_hour_ticks = true,
#else
  .short_tick_length = 4,
  .long_tick_length = 9,
  .corner_tick_length = 9,
  .bold_hour_ticks = false,
#endif
  .hand_inset = 28,
  .hand_tail = 18,
  .hub_radius = 7,
#endif
};

const PerimeterLayoutMetrics perimeter_layout_metrics = {
  .label_inset = 9,
#if defined(PBL_PLATFORM_EMERY)
  .tick_inset = 20,
#else
  .tick_inset = 18,
#endif
  // The hand radius is based on the smaller display dimension, even on rectangles.
  .hand_inset = 18,
#if defined(PBL_PLATFORM_GABBRO) || defined(PBL_PLATFORM_EMERY)
  .tick_length = 7,
  .hour_stroke = 3,
  .hand_tail = 18,
  .hub_radius = 7,
#else
  .tick_length = 5,
  .hour_stroke = 2,
  .hand_tail = 12,
  .hub_radius = 5,
#endif
};

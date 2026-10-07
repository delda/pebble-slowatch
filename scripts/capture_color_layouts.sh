#!/usr/bin/env bash

# Capture the watchface at 00:00 using each configured color layout.
# With no arguments, captures all default color-capable emulators.
#
# Examples:
#   scripts/capture_color_layouts.sh
#   scripts/capture_color_layouts.sh gabbro
set -euo pipefail

readonly PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly APP_FILE="$PROJECT_DIR/build/pebble-slowatch.pbw"
readonly TIME_SETTLE_SECONDS="${TIME_SETTLE_SECONDS:-2}"
readonly SCREENSHOT_TIMEOUT_SECONDS=30
# Generated from package.json: midnight_position=10000, color_layout=10001.
readonly COLOR_LAYOUT_MESSAGE_KEY=10001
readonly -a DEFAULT_PLATFORMS=(chalk flint gabbro)
readonly -a COLOR_LAYOUTS=(snow vanilla meadow sand sun mint sky forest default night)

emulator_logs_pid=""
emulator_logs_file=""

cd "$PROJECT_DIR"

platforms=("${DEFAULT_PLATFORMS[@]}")
if (( $# > 0 )); then
  platforms=("$@")
fi

stop_emulator() {
  if [[ -n "$emulator_logs_pid" ]]; then
    kill "$emulator_logs_pid" 2>/dev/null || true
    wait "$emulator_logs_pid" 2>/dev/null || true
    emulator_logs_pid=""
  fi

  if [[ -n "$emulator_logs_file" ]]; then
    rm -f "$emulator_logs_file"
    emulator_logs_file=""
  fi
}

start_emulator() {
  local platform="$1"
  local attempt

  pebble kill --force >/dev/null 2>&1 || true
  emulator_logs_file="$(mktemp)"
  pebble logs -vv --emulator "$platform" >"$emulator_logs_file" 2>&1 &
  emulator_logs_pid=$!

  for attempt in {1..30}; do
    if { rg -q 'Firmware booted\.' "$emulator_logs_file" &&
         rg -q 'pypkjs:Ready\.' "$emulator_logs_file"; } ||
       { rg -q 'QEMU is already running\.' "$emulator_logs_file" &&
         rg -q 'pypkjs is already running\.' "$emulator_logs_file"; }; then
      return 0
    fi

    if ! kill -0 "$emulator_logs_pid" 2>/dev/null; then
      break
    fi
    sleep 1
  done

  cat "$emulator_logs_file" >&2
  echo "Unable to start a ready $platform emulator." >&2
  return 1
}

install_watchface() {
  local platform="$1"
  local attempt

  for attempt in 1 2 3; do
    if pebble install --emulator "$platform" "$APP_FILE"; then
      return 0
    fi
    sleep 1
  done

  echo "Unable to install the watchface on $platform after three attempts." >&2
  return 1
}

set_emulator_time() {
  local platform="$1"
  local attempt

  for attempt in 1 2 3; do
    if pebble emu-set-time --emulator "$platform" --utc '00:00:00'; then
      return 0
    fi
    sleep 1
  done

  echo "Unable to set 00:00 on $platform after three attempts." >&2
  return 1
}

set_color_layout() {
  local platform="$1"
  local layout="$2"
  local attempt

  for attempt in 1 2 3; do
    if pebble send-app-message --emulator "$platform" \
        --string "$COLOR_LAYOUT_MESSAGE_KEY=$layout"; then
      return 0
    fi
    sleep 1
  done

  echo "Unable to set the $layout color layout on $platform after three attempts." >&2
  return 1
}

capture_screenshot() {
  local platform="$1"
  local frame="$2"
  local attempt

  for attempt in 1 2 3; do
    if timeout --foreground "$SCREENSHOT_TIMEOUT_SECONDS" \
        pebble screenshot --emulator "$platform" --no-open "$frame"; then
      return 0
    fi
    sleep 1
  done

  echo "Unable to capture $frame after three attempts." >&2
  return 1
}

create_color_layout_gif() {
  local output_dir="$1"
  local output_file="$output_dir/color-layouts.gif"

  if ! command -v ffmpeg >/dev/null 2>&1; then
    echo "ffmpeg is required to create $output_file." >&2
    return 1
  fi

  # The numbered filenames preserve the order of the ten layouts.
  ffmpeg -hide_banner -loglevel error -y \
    -framerate 2 \
    -pattern_type glob -i "$output_dir/layout-*.png" \
    -filter_complex '[0:v]split[frames][palette];[palette]palettegen=max_colors=256[p];[frames][p]paletteuse' \
    -loop 0 "$output_file"
}

capture_platform() {
  local platform="$1"
  local output_dir="$PROJECT_DIR/resources/images/screenshots/$platform/color-layouts"
  local index layout layout_number frame

  mkdir -p "$output_dir"
  rm -f "$output_dir"/layout-*.png

  start_emulator "$platform"
  install_watchface "$platform"
  pebble emu-button --emulator "$platform" click back || true
  set_emulator_time "$platform"
  sleep "$TIME_SETTLE_SECONDS"

  for index in "${!COLOR_LAYOUTS[@]}"; do
    layout="${COLOR_LAYOUTS[$index]}"
    printf -v layout_number '%03d' "$((index + 1))"
    frame="$output_dir/layout-$layout_number-$layout-00-00.png"
    set_color_layout "$platform" "$layout"
    sleep "$TIME_SETTLE_SECONDS"
    capture_screenshot "$platform" "$frame"
  done

  create_color_layout_gif "$output_dir"
  stop_emulator
  echo "Created 10 color-layout screenshots and $output_dir/color-layouts.gif"
}

trap stop_emulator EXIT

# The Pebble SDK requires Node v24.14.0 for the build.
source /home/delda/.nvm/nvm.sh
nvm use v24.14.0 >/dev/null
pebble build

for platform in "${platforms[@]}"; do
  capture_platform "$platform"
done

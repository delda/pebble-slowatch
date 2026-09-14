#!/usr/bin/env bash

# Capture this watchface at several times of day, once for each requested
# Pebble emulator. With no arguments, captures all platforms declared in
# package.json (currently chalk and gabbro).
#
# Examples:
#   scripts/capture_time_demo.sh
#   scripts/capture_time_demo.sh chalk
set -euo pipefail

readonly PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly APP_FILE="$PROJECT_DIR/build/pebble-slowatch.pbw"
# Override when the emulator is already responsive, e.g. TIME_SETTLE_SECONDS=1.
readonly TIME_SETTLE_SECONDS="${TIME_SETTLE_SECONDS:-2}"
# Flint's screenshot service stops responding on its 19th transfer in the same
# QEMU session. Leave one transfer of headroom and restart after 18 captures.
# Restarting periodically also keeps longer captures reliable on the other
# emulators.
readonly MAX_SCREENSHOTS_PER_EMULATOR_SESSION=18
readonly SCREENSHOT_TIMEOUT_SECONDS=30
# Keep the complete daily sequence short enough to share easily.  The actual
# frame rate is calculated from the number of captured PNGs below.
readonly GIF_MAX_DURATION_SECONDS=5

readonly -a DEFAULT_PLATFORMS=(chalk flint gabbro)
declare -a TIMES=()
for ((minutes=0; minutes < 24 * 60; minutes += 15)); do
  printf -v time '%02d:%02d' "$((minutes / 60))" "$((minutes % 60))"
  TIMES+=("$time")
done
readonly -a TIMES

emulator_logs_pid=""
emulator_logs_file=""

cd "$PROJECT_DIR"

platforms=("${DEFAULT_PLATFORMS[@]}")
if (( $# > 0 )); then
  platforms=("$@")
fi

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

create_time_demo_gif() {
  local output_dir="$1"
  local output_file="$output_dir/time-demo.gif"
  local frame_count gif_frame_rate
  local -a frames

  shopt -s nullglob
  frames=("$output_dir"/time-*.png)
  shopt -u nullglob
  frame_count="${#frames[@]}"

  if (( frame_count == 0 )); then
    echo "No PNG frames found to create $output_file." >&2
    return 1
  fi

  # Round up so all frames fit within the requested maximum duration. With
  # the default 96 captures this produces a 20 fps GIF (4.8 seconds).
  gif_frame_rate=$(( (frame_count + GIF_MAX_DURATION_SECONDS - 1) / GIF_MAX_DURATION_SECONDS ))

  if ! command -v ffmpeg >/dev/null 2>&1; then
    echo "ffmpeg is required to create $output_file." >&2
    return 1
  fi

  # The frame names sort chronologically, so the GIF progresses from midnight
  # through the full 24-hour rotation before looping back to the first frame.
  ffmpeg -hide_banner -loglevel error -y \
    -framerate "$gif_frame_rate" \
    -pattern_type glob -i "$output_dir/time-*.png" \
    -filter_complex '[0:v]split[frames][palette];[palette]palettegen=max_colors=256[p];[frames][p]paletteuse' \
    -loop 0 "$output_file"
}

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

  # Keeping `pebble logs` alive makes install, time changes, and screenshots
  # use the same QEMU instance. Without it, separate CLI calls can create
  # short-lived emulators whose clocks do not match the requested time.
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
  local time="$2"
  local attempt

  for attempt in 1 2 3; do
    if pebble emu-set-time --emulator "$platform" --utc "${time}:00"; then
      return 0
    fi
    sleep 1
  done

  echo "Unable to set $time on $platform after three attempts." >&2
  return 1
}

capture_platform() {
  local platform="$1"
  local output_dir="$PROJECT_DIR/resources/images/screenshots/$platform/time-demo"
  local time frame
  local capture_count=0

  mkdir -p "$output_dir"
  rm -f "$output_dir"/time-*.png

  start_emulator "$platform"
  install_watchface "$platform"
  # Return to the watchface if a diagnostic overlay is visible.
  pebble emu-button --emulator "$platform" click back || true

  for time in "${TIMES[@]}"; do
    frame="$output_dir/time-${time//:/-}.png"

    # The Flint emulator hangs on screenshot number 19 in a QEMU session.
    # Start a fresh session before reaching that limit, then restore the app
    # and requested time below.
    if (( capture_count > 0 && capture_count % MAX_SCREENSHOTS_PER_EMULATOR_SESSION == 0 )); then
      stop_emulator
      start_emulator "$platform"
      install_watchface "$platform"
      pebble emu-button --emulator "$platform" click back || true
    fi

    set_emulator_time "$platform" "$time"
    # The watchface redraws on the next minute tick after a time change.
    sleep "$TIME_SETTLE_SECONDS"
    capture_screenshot "$platform" "$frame"
    ((capture_count += 1))
  done

  create_time_demo_gif "$output_dir"
  stop_emulator
  echo "Created screenshots and $output_dir/time-demo.gif"
}

trap stop_emulator EXIT

# Build before starting the emulators so every capture uses the current app.
pebble build

for platform in "${platforms[@]}"; do
  capture_platform "$platform"
done

#!/usr/bin/env bash

# Capture each dial with the default colors and midnight at both positions,
# then combine the screenshots into a looping GIF.
# Pass the platform to capture on the requested emulator.
#
# Examples:
#   scripts/24h_dials.sh gabbro
set -euo pipefail

readonly PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly APP_FILE="$PROJECT_DIR/build/pebble-slowatch.pbw"
readonly TIME_SETTLE_SECONDS="${TIME_SETTLE_SECONDS:-2}"
readonly INSTALL_TIMEOUT_SECONDS="${INSTALL_TIMEOUT_SECONDS:-45}"
readonly SCREENSHOT_TIMEOUT_SECONDS=30
# Generated from package.json: midnight_position=10000, color_layout=10001, dial=10002.
readonly MIDNIGHT_POSITION_MESSAGE_KEY=10000
readonly COLOR_LAYOUT_MESSAGE_KEY=10001
readonly DIAL_MESSAGE_KEY=10002
# Keep platform validation aligned with the watchface's supported targets.
platform_list="$(python3 -c 'import json, sys; print("\n".join(json.load(open(sys.argv[1]))["pebble"]["targetPlatforms"]))' "$PROJECT_DIR/package.json")"
mapfile -t SUPPORTED_PLATFORMS <<< "$platform_list"
readonly -a SUPPORTED_PLATFORMS
readonly -a DIALS=(classic double)
readonly -a POSITIONS=(bottom top)

emulator_logs_pid=""
emulator_logs_file=""

cd "$PROJECT_DIR"

if (( $# != 1 )); then
  echo "Usage: $0 <platform>" >&2
  echo "Platforms: ${SUPPORTED_PLATFORMS[*]}" >&2
  exit 2
fi

platform="$1"
if [[ " ${SUPPORTED_PLATFORMS[*]} " != *" $platform "* ]]; then
  echo "Unsupported platform: $platform" >&2
  echo "Platforms: ${SUPPORTED_PLATFORMS[*]}" >&2
  exit 2
fi

if ! command -v ffmpeg >/dev/null 2>&1; then
  echo "ffmpeg is required to create the dial GIF." >&2
  exit 1
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
    # "Ready. Loaded apps" only means the cache was loaded. Wait until
    # pypkjs has actually connected to the firmware before installing.
    if { rg -q 'Firmware booted\.' "$emulator_logs_file" &&
         rg -q 'pypkjs\.pebble_manager:connected to' "$emulator_logs_file"; } ||
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
  echo "If the emulator shows a firmware crash, back up its SDK state before resetting it." >&2
  return 1
}

install_watchface() {
  local platform="$1"
  local attempt

  for attempt in 1 2; do
    echo "Installing watchface on $platform (attempt $attempt/2)..." >&2
    if timeout --foreground "$INSTALL_TIMEOUT_SECONDS" \
        pebble install --emulator "$platform" "$APP_FILE"; then
      return 0
    fi
    if (( attempt < 2 )); then
      echo "Install did not complete; retrying in 2 seconds." >&2
      sleep 2
    fi
  done

  echo "Unable to install the watchface on $platform within ${INSTALL_TIMEOUT_SECONDS}s per attempt." >&2
  if [[ -n "$emulator_logs_file" && -f "$emulator_logs_file" ]]; then
    echo "Emulator logs:" >&2
    cat "$emulator_logs_file" >&2
  fi
  return 1
}

send_setting() {
  local platform="$1"
  local key="$2"
  local value="$3"
  local attempt

  for attempt in 1 2 3; do
    if pebble send-app-message --emulator "$platform" --string "$key=$value"; then
      return 0
    fi
    sleep 1
  done

  echo "Unable to set $value (message key $key) on $platform after three attempts." >&2
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

capture_screenshot() {
  local platform="$1"
  local frame="$2"
  local attempt

  for attempt in 1 2; do
    echo "Capturing $(basename "$frame") (attempt $attempt/2)..." >&2
    if timeout --foreground "$SCREENSHOT_TIMEOUT_SECONDS" \
        pebble screenshot --emulator "$platform" --no-open "$frame"; then
      return 0
    fi
    if (( attempt < 2 )); then
      sleep 1
    fi
  done

  echo "Unable to capture $frame within ${SCREENSHOT_TIMEOUT_SECONDS}s per attempt." >&2
  return 1
}

create_dials_gif() {
  local output_dir="$1"
  local output_file="$output_dir/dials.gif"

  echo "Creating $output_file..." >&2
  # Numbered filenames preserve the capture order. Show each dial for a second.
  ffmpeg -hide_banner -loglevel error -y \
    -framerate 1 \
    -pattern_type glob -i "$output_dir/[0-9][0-9][0-9]-*.png" \
    -filter_complex '[0:v]split[frames][palette];[palette]palettegen=max_colors=256[p];[frames][p]paletteuse' \
    -loop 0 "$output_file"
}

capture_platform() {
  local platform="$1"
  local output_dir="$PROJECT_DIR/resources/images/screenshots/$platform/dials"
  local dial position frame frame_number
  local capture_index=0

  mkdir -p "$output_dir"
  rm -f "$output_dir"/*.png

  echo "Starting $platform emulator..." >&2
  start_emulator "$platform"
  install_watchface "$platform"
  pebble emu-button --emulator "$platform" click back || true
  # Explicitly select the configured default palette instead of relying on
  # whichever color layout may have been persisted by an earlier run.
  send_setting "$platform" "$COLOR_LAYOUT_MESSAGE_KEY" default
  set_emulator_time "$platform"
  sleep "$TIME_SETTLE_SECONDS"

  for position in "${POSITIONS[@]}"; do
    send_setting "$platform" "$MIDNIGHT_POSITION_MESSAGE_KEY" "$position"
    for dial in "${DIALS[@]}"; do
      echo "Capturing $dial dial with midnight at $position..." >&2
      send_setting "$platform" "$DIAL_MESSAGE_KEY" "$dial"
      sleep "$TIME_SETTLE_SECONDS"
      capture_index=$((capture_index + 1))
      printf -v frame_number '%03d' "$capture_index"
      frame="$output_dir/${frame_number}-${dial}-midnight-$position-00-00.png"
      capture_screenshot "$platform" "$frame"
    done
  done

  stop_emulator
  create_dials_gif "$output_dir"
  echo "Created 4 dial screenshots and $output_dir/dials.gif"
}

trap stop_emulator EXIT

# The Pebble SDK requires Node v24.14.0 for the build.
source /home/delda/.nvm/nvm.sh
nvm use v24.14.0 >/dev/null
echo "Building watchface..." >&2
pebble build

# pypkjs copies the PBW to a temporary file during installation. Use the
# project's filesystem: /tmp may reject writes because of a user disk quota,
# leaving pypkjs unable to send the install confirmation back to the CLI.
export TMPDIR="$PROJECT_DIR/build/screenshot-tmp"
mkdir -p "$TMPDIR"

capture_platform "$platform"

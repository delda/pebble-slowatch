# pebble-slowatch2

A Pebble watchapp/watchface written in C using the Pebble SDK.

## Building & running

```sh
pebble build                          # build for all targetPlatforms
pebble install --emulator emery       # install on the emery emulator
pebble install --phone <ip>           # install to a paired phone
```

## Target platforms

`targetPlatforms` in `package.json` controls which watches you build for. This
project supports **aplite**, **basalt**, **chalk**, **diorite**, **emery**,
**flint**, and **gabbro**. Aplite, Basalt (Pebble Time), Diorite (Pebble Time
Steel), and Flint share the 144×168 rectangular layout; Basalt and Diorite
render the navy, copper, and white palette in color. Emery (Pebble 2) uses the
rectangular 200×228 layout.

## Project layout

```
src/c/           C source for the watchapp
src/c/faces/     Dial implementations, including platform-specific layouts
src/pkjs/        PebbleKit JS (phone-side) source, if any
worker_src/c/    Background worker source, if any
resources/       Images, fonts, and other bundled resources
package.json     Project metadata (UUID, platforms, resources, message keys)
wscript          Build rules — usually no need to edit
```

`src/c/watch_face.c` selects the configured dial for drawing and its background
color. Classic (`src/c/faces/classic.c`) and Double (`src/c/faces/double.c`) share
the layout in `src/c/faces/classic_layout.c` for all supported platforms. Double
replaces each long mark with two aligned short marks separated by a gap. Each
segment uses the platform's normal short-mark length; number positions
are identical to Classic. Add future dials under `src/c/faces/` and register
them in the settings, configuration, and `watch_face.c` selection.

By default this project is configured as a watchapp. To make it a watchface,
set `pebble.watchapp.watchface` to `true` in `package.json`.

## Documentation

Full SDK docs, tutorials, and API reference: <https://developer.repebble.com>

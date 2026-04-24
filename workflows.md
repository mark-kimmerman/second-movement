# Workflows

- [Manage Fork](#manage-fork)
- [Build Firmware](#build-firmware)
- [Manage Watch Faces](#manage-watch-faces)
- [Compare with Upstream](#compare-with-upstream)

## Manage Fork

This repo is a fork of [joeycastillo/second-movement](https://github.com/joeycastillo/second-movement). Remotes and branches are managed as follows:

- `upstream/main` - joeycastillo's code (read-only)
- `origin/main`   - this fork's mirror (kept in sync with upstream)
- `origin/custom` - this fork's changes

### Setup on New Machine

```bash
git clone git@github.com:mark-kimmerman/second-movement.git
cd second-movement
git remote add upstream https://github.com/joeycastillo/second-movement.git
git fetch upstream
git checkout custom    # tracks origin/custom automatically
```

### Sync `main` with upstream

```bash
git fetch upstream
git checkout main
git merge --ff-only upstream/main    # --ff-only aborts if main has diverged
                                     # (keeps a clean mirror)
git push origin main
```

### Pull Updates into Working Branch

```bash
git checkout custom
git fetch upstream
git rebase upstream/main
git push --force-with-lease origin custom
```

## Build Firmware

Target hardware: Sensor Watch **`Pro`** board with the **`custom`** LCD.

Upstream build docs: <https://www.sensorwatch.net/docs/movement/building/>

### Install toolchain

One-time, per machine. Needed to cross-compile for the SAM L22.

- **macOS**: `brew install --cask gcc-arm-embedded`, or download the [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads/) installer and add it to `PATH`.
- **Linux (Debian/Ubuntu)**: `sudo apt install gcc-arm-none-eabi`.

Verify: `arm-none-eabi-gcc --version`.

### Sync submodules

Run before the first build, and any time upstream bumps a submodule ref:

```bash
git submodule update --init --recursive
```

### Build

```bash
make BOARD=sensorwatch_pro DISPLAY=custom
```

Output: `build/firmware.uf2`.

### Clean

Removes `build/` (and `build-sim/`) — all compiled output. Source and submodules are untouched. Useful before switching `BOARD`/`DISPLAY` or after pulling upstream changes that touch makefiles or headers.

```bash
make clean
```

### Flash to watch

1. Disassemble the watch and plug the board into USB.
2. Double-tap the tiny reset button on the back — LED pulses red.
3. Once the `WATCHBOOT` drive appears, run:

   ```bash
   make install
   ```

   This converts the build output to UF2 and copies it onto the mounted drive. You have ~1 minute before bootloader mode exits.

4. Or manually: drag `build/firmware.uf2` onto the `WATCHBOOT` drive.

### Build for simulator (browser)

Requires [emscripten](https://emscripten.org/) installed.

```bash
emmake make BOARD=sensorwatch_pro DISPLAY=custom
python3 -m http.server -d build-sim
```

Then open the printed localhost URL (e.g. <http://localhost:8000/firmware.html>)

### Useful optional flags

- `TIMESET=minute` — bakes the current time into the firmware so the watch boots with the clock already set.
- `NOSLEEP=1` — disables low-energy sleep mode. Handy when debugging so the display keeps updating instead of freezing after the inactivity timeout. Defines `-DMOVEMENT_LOW_ENERGY_MODE_FORBIDDEN`.

Examples:

```bash
# Bake in current time; prevent sleep
make BOARD=sensorwatch_pro DISPLAY=custom TIMESET=minute NOSLEEP=1

# Set time, then flash directly (double-tap reset first so WATCHBOOT is mounted)
make BOARD=sensorwatch_pro DISPLAY=custom TIMESET=minute install
```

## Manage Watch Faces

Movement shows a list of "watch faces" — screens cycled with the MODE button. The roster is compile-time; edit it and rebuild to change the watch's behavior.

Docs:

- Movement overview: <https://www.sensorwatch.net/docs/movement/>
- Designing a new watch face: <https://www.sensorwatch.net/docs/movement/newface/>

### Change the set or order of faces

Edit the `watch_faces[]` array in [`movement_config.h`](./movement_config.h) (around line 30). Order = cycle order when pressing MODE.

```c
const watch_face_t watch_faces[] = {
    clock_face,                 // index 0 — the "home" face
    world_clock_face,
    sunrise_sunset_face,
    // ... add, remove, or reorder entries here ...
    settings_face,
    set_time_face,
};
```

Notes:

- A face is only available if its header is `#include`d in [`movement_faces.h`](./movement_faces.h). All faces shipped with the repo are already included; the template script (below) keeps this up to date for new faces.
- `MOVEMENT_SECONDARY_FACE_INDEX` in the same file splits the list into a primary group (short-press MODE cycles through it) and a secondary group (long-press MODE jumps there). Useful for pushing settings/utility faces behind a long-press.
- Rebuild and flash after editing: see [Build Firmware](#build-firmware).

### Create a new watch face

Use the generator in [`template/`](./template/) — it stamps out the `.c`/`.h` boilerplate and wires the new face into `movement_faces.h` and `watch-faces.mk`.

```bash
cd template
python3 watch_face.py <face_type> <face_name> --author-name "Your Name"
```

- `<face_type>` — one of `clock`, `complication`, `demo`, `sensor`, `settings`. Determines which subdirectory of `watch-faces/` the files land in.
- `<face_name>` — lowercase, underscores between words (e.g. `pomodoro_timer`).

The script:

- creates `watch-faces/<face_type>/<face_name>_face.c` and `.h`
- adds `#include "<face_name>_face.h"` to `movement_faces.h` (above the `// New includes go above this line.` marker)
- adds the new `.c` path to `watch-faces.mk` (above the `# New watch faces go above this line.` marker)

You still need to:

1. Implement `setup` / `activate` / `loop` / `resign` in the generated `.c`. Start by reading [`watch-faces/clock/simple_clock_face.c`](./watch-faces/clock/simple_clock_face.c) for a minimal real example, and [`template/template.c`](./template/template.c) for the annotated boilerplate.
2. Register the face in `watch_faces[]` in `movement_config.h` (see above).
3. Rebuild and flash: see [Build Firmware](#build-firmware).

Full walkthrough and API reference: <https://www.sensorwatch.net/docs/movement/newface/>.

## Compare with Upstream

See what your `custom` branch has that `upstream/main` doesn't. Always fetch first so you're comparing against the latest:

```bash
git fetch upstream
```

### `..` vs `...` — the important distinction

- `upstream/main..custom` (two dots) — commits reachable from `custom` but not `upstream/main`. Use with `git log`.
- `upstream/main...custom` (three dots) — for `git diff`, this means "diff from the merge base to `custom`" → **only your changes**, even if upstream has moved on. Use with `git diff`.

Using plain `..` with `git diff` mixes in upstream changes you haven't merged yet, which is almost never what you want.

### List files you've changed

```bash
git diff --name-only upstream/main...custom
```

Add `--name-status` instead of `--name-only` to see `M`/`A`/`D` for modified/added/deleted.

### Summary of changes

```bash
git diff --stat upstream/main...custom
```

One line per file with insert/delete counts.

### Full diff

```bash
git diff upstream/main...custom
```

Pipe through a pager or into a file if it's large:

```bash
git diff upstream/main...custom > my-changes.diff
```

### Diff a single file or directory

```bash
git diff upstream/main...custom -- movement_config.h
git diff upstream/main...custom -- watch-faces/
```

### List your commits (not in upstream)

```bash
git log upstream/main..custom --oneline
```

Drop `--oneline` for full commit messages. Add `--stat` to see files touched per commit.

### Quick visual overview

Open the GitHub compare view in a browser:

<https://github.com/joeycastillo/second-movement/compare/main...mark-kimmerman:second-movement:custom>



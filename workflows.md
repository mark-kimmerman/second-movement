# Workflows

- [Manage Fork](#manage-fork)
- [Build Firmware](#build-firmware)

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

Then open the printed localhost URL.

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


# MuppetEngine

Arduino sketch for a puppet hat that animates two servo-driven arms. Targets the **Seeed Studio XIAO RA4M1**.

## Hardware

| Pin | Function |
|-----|----------|
| D9  | Left servo PWM signal |
| D10 | Right servo PWM signal |
| D8  | Left servo power-gate transistor (optional) |
| D7  | Right servo power-gate transistor (optional) |

- Servo signal pins are 3.3V logic — power servo motors from an external 5V supply sharing GND with the XIAO, not from the board's 3.3V rail.
- Power gating (D7/D8) cuts motor current when arms are at rest to save battery. If not wired, leave those pins unconnected — the gate writes are harmless and servos stay always-powered.

## Behavior

- Poisson-distributed random intervals between wave events (`MEAN_INTERVAL_SEC`)
- Each event randomly picks one arm to wave for 3–8 cycles
- 5% chance of `spazArms()` instead — both arms thrash up/down for 15 cycles (Kermit mode)
- Arms always return to `REST_ANGLE` before motors are gated off

## Key tuning constants (top of sketch)

```cpp
REST_ANGLE        = 15;    // arm-down resting position — tune to your servo mounting
WAVE_ANGLE        = 160;   // arm-up position — tune to your servo mounting
WAVE_STEP_MS      = 6;     // ms per degree; lower = faster wave
SPAZ_HOLD_MS      = 80;    // ms at each extreme during spaz
MEAN_INTERVAL_SEC = 8.0f;  // mean seconds between events
SPAZ_CHANCE_PCT   = 5;     // % chance of spaz vs normal wave
```

## Toolchain

Board support is installed via **arduino-cli** (not the Arduino IDE board manager or PlatformIO). The Seeed Renesas core is at `Seeeduino:renesas_uno` and the board FQBN is `Seeeduino:renesas_uno:XIAO_RA4M1`.

Board manager URL used:
```
https://files.seeedstudio.com/arduino/package_renesas_1.2.0_index.json
```

### Flash one-liner

```bash
arduino-cli compile --fqbn Seeeduino:renesas_uno:XIAO_RA4M1 ~/Documents/Code/ArduinoBackup/MuppetEngine && arduino-cli upload -p /dev/cu.usbmodem2101 --fqbn Seeeduino:renesas_uno:XIAO_RA4M1 ~/Documents/Code/ArduinoBackup/MuppetEngine
```

> The port `/dev/cu.usbmodem2101` may change — run `arduino-cli board list` to find it if the upload fails.

### Serial monitor

```bash
stty -f /dev/cu.usbmodem2101 115200 && cat /dev/cu.usbmodem2101
```

Serial output (115200 baud) prints `MuppetEngine starting...`, the time until the first event, then `Wave LEFT/RIGHT xN` or `SPAZ ARMS` each time an action fires. Ctrl+C to stop.

> If the monitor is silent, check `lsof /dev/cu.usbmodem2101` — PlatformIO's device monitor (python3) locks the port and eats the output.

## PlatformIO note

PlatformIO works but uploads a different binary than arduino-cli (different platform source). Stick to arduino-cli for consistency. If using PlatformIO, close its device monitor before reading serial with `cat` — it holds an exclusive lock on the port.

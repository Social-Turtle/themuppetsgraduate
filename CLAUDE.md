# MuppetEngine

Arduino sketch for a puppet hat that animates two servo-driven arms. Targets the **Seeed Studio XIAO RA4M1**.

## Hardware

| Pin | Function |
|-----|----------|
| D9  | Servo PWM signal |
| D8  | Servo power-gate transistor (optional) |

- Servo signal pin is 3.3V logic — power servo motor from an external 5V supply sharing GND with the XIAO, not from the board's 3.3V rail.
- Power gating (D8) cuts motor current when arm is at rest to save battery. If not wired, leave D8 unconnected — the gate write is harmless and servo stays always-powered.

## Behavior

- Poisson-distributed random intervals between wave events (`MEAN_INTERVAL_SEC`)
- Each event waves for 2–5 randomly chosen cycles; each cycle (up + down) takes 0.9 seconds
- Arm always returns to `REST_ANGLE` before motor is gated off

## Key tuning constants (top of sketch)

```cpp
REST_ANGLE        = 45;    // arm resting position — 45° from straight down
WAVE_ANGLE        = 135;   // arm-up position — 90° of travel from REST_ANGLE
WAVE_STEP_MS      = 5;     // ms per degree; 90° each way × 2 = 900ms/cycle
MEAN_INTERVAL_SEC = 8.0f;  // mean seconds between events
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

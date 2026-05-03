# nRF5340 Secure Sensor Node

A Zephyr RTOS-based secure sensor node built on the Nordic nRF5340 DK.

This project demonstrates an embedded IoT security-oriented sensor node with Bluetooth Low Energy telemetry, interrupt-driven tamper detection, OLED display integration over I2C, software debounce, and persistent flash storage using Zephyr NVS.

## Project Overview

The goal of this project is to build a small embedded security-oriented sensor node that can detect tamper-style events, expose runtime state over BLE, provide UART debug logs, and preserve important state across reboots.

The project was developed incrementally through milestones, starting from basic firmware boot and growing into a more realistic embedded system.

## Hardware Used

- Nordic nRF5340 DK
- SSD1306 128x32 OLED display over I2C
- Onboard nRF5340 DK button used as a tamper input
- Adafruit LSM6DSOX IMU breakout planned for motion detection
- Jumper wires, breadboard, STEMMA QT / Qwiic accessories

## Software Stack

- Nordic nRF Connect SDK
- Zephyr RTOS
- Bluetooth Low Energy
- Custom BLE GATT service
- GPIO interrupt handling
- I2C peripheral configuration
- SSD1306 display driver
- Zephyr NVS flash storage
- UART logging and debugging

## Current Features

- Zephyr firmware running on nRF5340 DK
- BLE advertising
- Custom BLE heartbeat characteristic
- UART heartbeat logging
- Button-based tamper event detection
- Software debounce for mechanical button bounce
- OLED bring-up over I2C
- OLED update loop integrated into runtime
- I2C TWIM buffer configuration fix
- Persistent tamper counter using Zephyr NVS
- Tamper count survives reboot/reset

## Implemented Milestones

### M1 - Basic Boot

Verified that the firmware boots correctly on the Nordic nRF5340 DK and prints UART output.

### M2 - BLE Heartbeat

Added Bluetooth Low Energy support with a custom 128-bit GATT service and heartbeat characteristic.

### M3 - OLED Bring-up

Integrated an SSD1306 128x32 OLED display over I2C. The display was wired to the nRF5340 DK and configured through a Zephyr devicetree overlay.

### M4 - Button-Based Tamper Detection

Used the onboard button as a bench-test tamper input. Each button press triggers an interrupt callback and increments a tamper counter.

### M5 - Software Debounce

Added software debounce logic to reduce repeated tamper events caused by mechanical button bounce.

### M6 - OLED Runtime Update Loop

Integrated an OLED update function into the runtime loop and fixed a TWIM internal buffer issue by increasing the I2C concat buffer size in the devicetree overlay.

### M7 - Flash Persistence with NVS

Added persistent storage for the tamper counter using Zephyr NVS. The device now saves the tamper counter to flash and restores it after reboot.

## Repository Structure

```text
apps/secure_sensor_node/
├── boards/
│   └── nrf5340dk_nrf5340_cpuapp.overlay
├── src/
│   └── main.c
├── CMakeLists.txt
├── prj.conf
└── pm_static.yml
```

## Build Instructions

From the repository root:

```bash
west build -b nrf5340dk/nrf5340/cpuapp apps/secure_sensor_node --pristine
```

## Flash Instructions

```bash
west flash
```

## Expected UART Output

```text
[UART] BOOT: secure_sensor_node
[UART] Loaded tamper_count=2 from flash
[UART] Heartbeat 10 | Tamper 2
[UART] Tamper event! count=3
[UART] Saved tamper_count=3 to flash
```

## Key Problems Solved

During development, several real embedded issues were encountered and fixed:

- Incorrect Zephyr board target naming
- Missing devicetree pinctrl configuration
- I2C peripheral instance conflict
- Unsupported SSD1306 Kconfig option
- Unstable OLED wiring during early testing
- Poor OLED module readability
- I2C TWIM internal buffer size error
- Mechanical button bounce
- Persistent storage partition setup

## Key Embedded Concepts Practiced

- Zephyr project structure
- Nordic nRF5340 board target configuration
- Devicetree overlays
- I2C pin configuration
- BLE GATT service implementation
- GPIO interrupts
- Debounce logic
- Flash memory persistence
- NVS storage
- UART debugging
- Incremental firmware development 
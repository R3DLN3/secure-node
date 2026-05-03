# Hardware Documentation

## Main Development Board

Nordic nRF5340 DK

Used for:
- Zephyr firmware development
- BLE peripheral application
- GPIO input
- I2C peripherals
- UART logging
- flash/NVS storage

## OLED Display

Type:
- SSD1306
- 128x32
- I2C

Connections:

| OLED Pin | nRF5340 DK |
|---|---|
| SDA | P0.26 |
| SCL | P0.25 |
| GND | GND |
| VIN | 3.3V |

Notes:
- The OLED module required soldering header pins.
- The module electrically responds to firmware writes.
- Visual clarity of the current OLED module is limited.
- Display bring-up was still successful from a firmware and hardware integration perspective.

## I2C Bus

The project uses I2C1 instead of I2C0.

Reason:
- I2C0 caused a Nordic peripheral instance conflict with other enabled peripherals.
- Moving to I2C1 resolved the conflict.

Configured pins:
- SDA: P0.26
- SCL: P0.25

## Tamper Input

Current implementation:
- onboard nRF5340 DK button
- devicetree alias: `sw0`

Purpose:
- used as a bench-test version of a real tamper switch

Behavior:
- pressing the button triggers a GPIO interrupt
- the interrupt callback increments `tamper_count`
- the value is printed over UART
- the value is saved to flash using NVS

## Flash Storage

Used for:
- persistent tamper counter

Storage method:
- Zephyr NVS

Partition:
- defined in `pm_static.yml`

## Planned IMU

Sensor:
- Adafruit LSM6DSOX breakout

Planned use:
- motion detection
- shock detection
- movement events

Interface:
- I2C

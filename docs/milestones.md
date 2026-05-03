# Milestone History



This document tracks the development milestones of the nRF5340 Secure Sensor Node project.



The project was built incrementally, starting from a basic Zephyr firmware boot and gradually adding BLE, GPIO tamper detection, OLED integration, debounce logic, and flash persistence.







## M1 - Basic Firmware Boot



Goal:  

Verify that the Nordic nRF5340 DK can boot custom Zephyr firmware and print basic UART output.



Completed:

- Created the `secure_sensor_node` application.

- Built firmware for `nrf5340dk/nrf5340/cpuapp`.

- Flashed the board successfully.

- Verified UART boot output.



Result:  

The application boots successfully and prints serial output.





## M2 - BLE Heartbeat



Goal:  

Add Bluetooth Low Energy support and expose a basic runtime heartbeat value.



Completed:

- Enabled Bluetooth LE peripheral mode.

- Created a custom 128-bit BLE service.

- Created a heartbeat characteristic.

- Added BLE advertising.

- Printed heartbeat counter over UART.

- Added BLE notification support when enabled by a connected client.



Result:  

The node advertises over BLE and maintains a runtime heartbeat counter.





## M3 - OLED Bring-up



Goal:  

Connect and initialize an SSD1306 128x32 OLED display over I2C.



Completed:

- Soldered header pins onto the OLED module.

- Connected OLED to the nRF5340 DK over I2C.

- Configured I2C using a Zephyr devicetree overlay.

- Resolved missing `pinctrl-0` devicetree error.

- Moved from `i2c0` to `i2c1` due to Nordic peripheral instance conflict.

- Initialized the SSD1306 display driver.

- Verified that the OLED responds to firmware writes.



Notes:

- The OLED module works electrically.

- The current OLED module has limited visual clarity.

- Display bring-up is still considered successful because the firmware can initialize and write to the display.



Result:  

SSD1306 OLED bring-up completed over I2C.





## M4 - Button-Based Tamper Detection



Goal:  

Use the onboard nRF5340 DK button as a simulated tamper input.



Completed:

- Enabled GPIO support.

- Used Zephyr devicetree button alias `sw0`.

- Configured the button as a GPIO input.

- Enabled GPIO interrupt handling.

- Added an interrupt callback.

- Incremented `tamper_count` on each valid button event.

- Logged tamper events over UART.



Result:  

Pressing the onboard button triggers tamper events and increments the tamper counter.





## M5 - Software Debounce



Goal:  

Prevent multiple tamper counts from a single physical button press.



Completed:

- Added uptime-based debounce logic.

- Ignored button events inside a short debounce window.

- Improved tamper counter stability.

- Reduced false multiple events caused by mechanical switch bounce.



Result:  

Button input became more stable and suitable for tamper-style event detection.





## M6 - OLED Runtime Update Loop and I2C Buffer Fix



Goal:  

Integrate OLED update logic into the runtime loop and fix I2C transfer buffer issues.



Completed:

- Added periodic OLED update function.

- Integrated OLED update into the main runtime loop.

- Encountered TWIM internal buffer size error during display writes.

- Fixed the issue by increasing `zephyr,concat-buf-size` in the I2C node of the overlay.



Problem solved:  

The SSD1306 display write attempted a larger I2C transfer than the default TWIM internal buffer allowed.



Fix:  

Increased the I2C concat buffer size in the devicetree overlay.



Result:  

The OLED update loop runs without repeated TWIM internal buffer errors.





## M7 - Flash Persistence with Zephyr NVS



Goal:  

Persist the tamper counter across resets and reboots.



Completed:

- Enabled flash support.

- Enabled flash map support.

- Enabled Zephyr NVS.

- Created `pm_static.yml`.

- Defined a `storage_partition`.

- Mounted NVS during boot.

- Loaded saved `tamper_count` from flash.

- Saved updated `tamper_count` after valid tamper events.



Result:  

The tamper counter survives reset and reboot.



Example behavior:

- Pressing the button increments `tamper_count`.

- The value is saved to flash.

- After reboot, the saved tamper count is restored.







## M8 - Planned Motion Detection



Goal:  

Add real sensor-based motion detection using the Adafruit LSM6DSOX IMU.



Status:  

Planned / next development step.



Planned work:

- Connect LSM6DSOX over I2C.

- Configure the sensor node in the devicetree overlay.

- Enable the Zephyr sensor driver.

- Poll accelerometer and gyroscope data.

- Detect movement, shock, or tilt events.

- Log motion events over UART.

- Later expose motion data over BLE.






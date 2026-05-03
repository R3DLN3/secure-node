\# Testing Documentation



This document describes how the nRF5340 Secure Sensor Node project was tested during development.



\---



\## 1. Build Test



Command:



```bash

west build -b nrf5340dk/nrf5340/cpuapp apps/secure\_sensor\_node --pristine

```



Expected result:



```text

Linking C executable zephyr/zephyr.elf

Generating ../merged.hex

```



Purpose:



This verifies that the Zephyr application, board target, devicetree overlay, Kconfig options, and source code all compile successfully.



\---



\## 2. Flash Test



Command:



```bash

west flash

```



Expected result:



```text

Board flashed successfully

```



Purpose:



This verifies that the generated firmware can be programmed to the Nordic nRF5340 DK.



\---



\## 3. UART Boot Test



Expected output after reset:



```text

\[UART] BOOT: secure\_sensor\_node

```



Purpose:



This confirms that the firmware starts correctly and that UART logging is working.



\---



\## 4. Heartbeat Test



Expected output:



```text

\[UART] Heartbeat 1 | Tamper 0

\[UART] Heartbeat 2 | Tamper 0

\[UART] Heartbeat 3 | Tamper 0

```



Purpose:



This confirms that the main loop is running and that the firmware remains alive over time.



\---



\## 5. Tamper Button Test



Action:



Press the onboard button used as the tamper input.



Expected output:



```text

\[UART] Tamper event! count=1

\[UART] Heartbeat 10 | Tamper 1

```



Purpose:



This verifies GPIO input handling, interrupt callbacks, and tamper event counting.



\---



\## 6. Debounce Test



Action:



Press the button normally once.



Expected behavior:



A normal press should produce one valid tamper event instead of several rapid repeated counts.



Purpose:



This validates the software debounce logic used to reduce mechanical switch bounce.



\---



\## 7. OLED Bring-up Test



Expected behavior:



\- OLED receives power.

\- OLED responds to firmware writes.

\- Display driver initializes successfully.

\- OLED update loop runs without crashing the firmware.



Known limitation:



The current low-cost SSD1306 OLED module has poor visual readability, but the module does respond electrically and the firmware can write to it.



Purpose:



This verifies I2C display bring-up and SSD1306 driver integration.



\---



\## 8. I2C TWIM Buffer Fix Test



Previous problem:



```text

Need to use the internal driver buffer but its size is insufficient

```



Expected after fix:



The repeated TWIM internal buffer error should no longer appear during OLED update writes.



Purpose:



This confirms that `zephyr,concat-buf-size` is correctly configured in the I2C devicetree node.



\---



\## 9. Flash Persistence Test



Steps:



1\. Flash the firmware.

2\. Open UART logs.

3\. Press the tamper button several times.

4\. Confirm that `tamper\_count` increases.

5\. Reset or reboot the board.

6\. Check the boot logs.



Expected output after reboot:



```text

\[UART] Loaded tamper\_count=3 from flash

```



Purpose:



This verifies that Zephyr NVS is saving and restoring the tamper counter correctly.



\---



\## 10. BLE Advertising Test



Action:



Open a BLE scanner application such as nRF Connect on a phone.



Expected behavior:



The device should advertise using the configured BLE device name.



Purpose:



This verifies that BLE advertising starts successfully and that the node can be discovered by a BLE client.



\---



\## 11. BLE Heartbeat Characteristic Test



Action:



Connect to the device using a BLE scanner application and inspect the custom GATT service.



Expected behavior:



The heartbeat characteristic should be readable, and notifications may be received if enabled by the client.



Purpose:



This verifies the custom BLE GATT service and heartbeat characteristic implementation.



\---



\## Test Summary



The project has been tested through:



\- Zephyr build and flash workflow

\- UART boot logging

\- runtime heartbeat logging

\- GPIO interrupt-based tamper detection

\- software debounce behavior

\- OLED display initialization

\- I2C TWIM buffer fix validation

\- flash persistence using Zephyr NVS

\- BLE advertising and GATT heartbeat functionality



These tests confirm that the project functions as an embedded IoT security node prototype.


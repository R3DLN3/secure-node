\# Troubleshooting Documentation



This document lists the main technical issues encountered during development of the nRF5340 Secure Sensor Node project and how they were solved.



\---



\## 1. Incorrect Zephyr Board Name



\### Problem



The build failed when using an underscore-based board name:



```bash

west build -b nrf5340dk\_nrf5340\_cpuapp

```



\### Cause



The installed NCS/Zephyr version expects the board name using slash qualifiers.



\### Fix



Use:



```bash

west build -b nrf5340dk/nrf5340/cpuapp apps/secure\_sensor\_node --pristine

```



\### Result



The project configured correctly for the Nordic nRF5340 DK application core.



\---



\## 2. Build Directory Board Mismatch



\### Problem



The build directory was already configured for another board target and produced an error similar to:



```text

Build directory targets board ..., but board ... was specified

```



\### Cause



Zephyr keeps board configuration inside the build directory. When the board target changes or the build directory was created with another target, the build can fail.



\### Fix



Use a pristine build:



```bash

west build -b nrf5340dk/nrf5340/cpuapp apps/secure\_sensor\_node --pristine

```



\### Result



The old build configuration was cleared and the project rebuilt cleanly.



\---



\## 3. Missing I2C pinctrl Configuration



\### Problem



The devicetree build failed with an error similar to:



```text

'pinctrl-0' is marked as required

```



\### Cause



The I2C node was enabled without defining the required pin control settings for SDA and SCL.



\### Fix



Added full `pinctrl` configuration in the board overlay for:



```text

SDA = P0.26

SCL = P0.25

```



\### Result



Zephyr could correctly map I2C signals to physical nRF5340 pins.



\---



\## 4. I2C0 Peripheral Conflict



\### Problem



The build failed with a Nordic peripheral conflict:



```text

Only one of the following peripherals can be enabled: SPI0, SPIM0, SPIS0, TWI0, TWIM0, TWIS0, UARTE0

```



\### Cause



The selected I2C instance conflicted with another enabled Nordic peripheral instance.



\### Fix



Moved the OLED I2C configuration from `i2c0` to `i2c1`.



\### Result



The peripheral conflict was resolved while keeping the same physical SDA/SCL pins.



\---



\## 5. Unsupported SSD1306 Kconfig Symbol



\### Problem



The build failed with:



```text

attempt to assign the value 'y' to the undefined symbol SSD1306\_DEFAULT

```



\### Cause



`CONFIG\_SSD1306\_DEFAULT` is not supported in the current Zephyr/NCS version used for this project.



\### Fix



Removed:



```conf

CONFIG\_SSD1306\_DEFAULT=y

```



Kept:



```conf

CONFIG\_DISPLAY=y

CONFIG\_SSD1306=y

```



\### Result



The SSD1306 driver configuration built correctly.



\---



\## 6. OLED Wiring Instability



\### Problem



The OLED display reacted but output was unstable or unclear during early tests.



\### Cause



Loose jumper wires and mechanically weak connections caused unreliable display behaviour.



\### Fix



\- Soldered header pins to the OLED module.

\- Improved physical connections.

\- Tested using breadboard wiring.

\- Verified I2C communication and display response.



\### Result



The OLED could be initialized and written to by firmware.



\---



\## 7. Poor OLED Visual Clarity



\### Problem



The SSD1306 module responded, but the displayed output was visually unclear.



\### Cause



The display driver and I2C communication worked, but the specific low-cost OLED module had limited readability.



\### Fix / Decision



The OLED was still considered successfully brought up because the firmware could initialize and write to the display. The visual clarity limitation was documented as a hardware/module quality issue rather than a complete firmware failure.



\### Result



OLED bring-up was counted as successful, with a note that a better display module may be used later.



\---



\## 8. TWIM Internal Buffer Size Error



\### Problem



Runtime logs showed repeated errors:



```text

Need to use the internal driver buffer but its size is insufficient

```



\### Cause



The SSD1306 display write attempted a larger I2C transfer than the default Nordic TWIM internal buffer allowed.



\### Fix



Added this property to the `i2c1` node in the overlay:



```dts

zephyr,concat-buf-size = <600>;

```



\### Result



The repeated I2C TWIM internal buffer error disappeared.



\---



\## 9. Mechanical Button Bounce



\### Problem



One physical button press sometimes produced multiple tamper events.



\### Cause



Mechanical switches can bounce electrically, causing several fast edges for a single physical press.



\### Fix



Added software debounce using Zephyr uptime timing:



```c

int64\_t now = k\_uptime\_get();



if (now - last\_press\_time < 200) {

&#x20;   return;

}



last\_press\_time = now;

```



\### Result



Button input became more stable and more suitable for tamper-style event detection.



\---



\## 10. Flash Persistence Setup



\### Problem



The project needed persistent storage so that `tamper\_count` would survive reset/reboot.



\### Cause



RAM variables are lost after reset, so persistent state requires flash storage.



\### Fix



Enabled Zephyr flash/NVS support and created:



```text

pm\_static.yml

```



With:



```yml

storage\_partition:

&#x20; address: 0xF4000

&#x20; size: 0x0C000

&#x20; region: flash\_primary

```



\### Result



The tamper counter can now be saved and restored using Zephyr NVS.



\---



\## 11. GitHub Documentation and Repository Cleanup



\### Problem



The working project folder and GitHub folder were not initially aligned, and some generated/build files should not be committed.



\### Cause



The active Zephyr project existed in:



```text

C:\\Users\\Usman\\secure-node

```



while another GitHub folder existed under OneDrive. The working project folder was confirmed as the correct source.



\### Fix



\- Confirmed the real working repo path.

\- Added the working source files to Git.

\- Renamed `pm\_static.yml.txt` to `pm\_static.yml`.

\- Prepared documentation files under `docs/`.

\- Ensured build artifacts are ignored through `.gitignore`.



\### Result



The repository became cleaner and better structured for GitHub presentation.



\---



\## Summary



This project involved real embedded debugging across:



\- Zephyr build system

\- Nordic board target naming

\- Devicetree overlays

\- I2C pinctrl configuration

\- I2C peripheral conflicts

\- SSD1306 display bring-up

\- Nordic TWIM buffer sizing

\- GPIO interrupts

\- Software debounce

\- Flash/NVS persistence

\- GitHub documentation workflow



These issues helped turn the project from a simple firmware example into a more realistic embedded systems portfolio project.


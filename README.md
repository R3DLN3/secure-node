# secure-node

Two embedded + security portfolio projects on the nRF5340 DK (Zephyr / nRF Connect SDK).

## Projects
## 1) Secure Sensor Node
- IMU sampling + simple motion classification
- BLE GATT exposes metrics + counters
- OLED shows live state
- Clean architecture: drivers / services / app

## 2) Tamper-Evident Incident Logger (Hardware-Backed)
- IMU triggers tamper events
- Event log stored on device
- Each entry signed with ATECC608A (Adafruit 4314)
- Laptop script verifies log integrity
- OLED shows TAMPER + signed count

## Repo structure
- `apps/` firmware apps
- `modules/` shared drivers/services/common code
- `scripts/` host-side scripts
- `docs/` documentation + diagrams

## Toolchain
Built with nRF Connect SDK (Zephyr-based) and flashed with `west`.

# Micro OLED Eyes

A dual-display hardware project using a Seeed Studio XIAO RP2040 and two SH1106 OLED screens to animate synchronized eye tracking movements.

---

## Hardware Configuration & Pin Mapping

Because the silk-screened labels on the XIAO RP2040 do not match the native internal RP2040 GPIO numbers, use the following mapping for wiring and firmware configuration:

| OLED Pin Name | Function | Display 1 (Left Eye) | Display 2 (Right Eye) | XIAO Silk Label | Native GPIO Number |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **GND** | Ground | GND | GND | **GND** | **GND** |
| **VCC** | Power (3.3V) | VCC | VCC | **3V3** | **3V3** |
| **D0** | SPI Clock | SCLK | SCLK | **D8** | **GPIO 2** |
| **D1** | SPI Data (MOSI) | MOSI | MOSI | **D10** | **GPIO 3** |
| **RES** | Hardware Reset | RES | RES | **D3** | **GPIO 29** |
| **DC** | Data / Command | DC | DC | **D4** | **GPIO 6** |
| **CS** | Chip Select | CS | — | **D1** | **GPIO 27** |
| — | Chip Select | — | CS | **D2** | **GPIO 28** |

> **Note:** The hardware reset line (`RES`) is shared between both displays. To prevent the initialization of one screen from wiping out the other, the reset cycle is managed manually in the firmware setup sequence before booting the software buffers.

## Firmware & Development Environment

This project is built using **PlatformIO** inside Antigravity IDE (or VS Code), utilizing the community-maintained Arduino-Pico core for optimal RP2040 support.

### Dependencies
The following libraries are required and managed automatically via `platformio.ini`:
* **U8g2** (by olikraus) - For high-performance monochrome display driving.

### Compilation & Uploading
1. Open this project directory in your IDE with the PlatformIO extension installed.
2. Connect the Seeed Studio XIAO RP2040 via USB-C.
3. Click the **Build** button (✓) to compile and download dependencies.
4. Click the **Upload** button (→) to flash the binary to the microcontroller.
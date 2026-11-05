# STM32 Bootloader & Firmware Updater

A configurable STM32 bootloader project with a standalone Windows desktop application for firmware updates, flash memory management, and device communication.

The project supports STM32F407 and STM32F746 microcontrollers using a custom binary communication protocol over USB CDC and UART.

## Features

### STM32 Bootloader

- STM32F407 and STM32F746 support
- Configurable MCU selection
- USB CDC and UART communication
- Configurable UART interface, baud rate, and GPIO pins
- Bootloader entry using a configurable GPIO pin
- Firmware update using binary `.bin` files
- Flash sector erase and programming
- CRC32-based firmware integrity verification
- Application vector table validation
- Bootloader-to-application jump
- Device reset and flash memory operations

### Windows Firmware Updater

- Standalone Windows executable
- Graphical user interface
- USB CDC and UART connection through COM ports
- Automatic serial port detection
- Firmware file selection
- Upload progress monitoring
- Flash memory erase and verification
- Bootloader version and MCU identification
- Memory reading and CRC32 calculation
- Device reset and application jump commands
- Real-time operation logs

## Supported Microcontrollers

| Microcontroller | Family | Application Address |
|---|---|---|
| STM32F407 | STM32F4 | `0x08020000` |
| STM32F746 | STM32F7 | `0x08020000` |

The current project uses a 128 KB bootloader region starting at `0x08000000`.

Memory allocation and flash operations must match the selected target MCU and application linker script.

## Communication Protocol

The bootloader implements a custom binary communication protocol.

Packet structure:

`Header (AA 55) | Length | Command | Sequence | Payload | CRC32`

Available commands:

| ID | Command | Description |
|---|---|---|
| 1 | GET_VERSION | Read bootloader version |
| 2 | GET_HELP | List available commands |
| 3 | GET_CHIP_ID | Read microcontroller identification |
| 4 | GET_STATUS | Read bootloader status |
| 5 | START_UPDATE | Initialize firmware update |
| 6 | WRITE_CHUNK | Write firmware data |
| 7 | FINISH_UPDATE | Finalize firmware update |
| 8 | ABORT_UPDATE | Cancel firmware update |
| 9 | JUMP_TO_APP | Start user application |
| 10 | GET_RDP_STATUS | Read protection status |
| 11 | FLASH_ERASE | Erase application flash |
| 12 | MEM_READ | Read device memory |
| 13 | GET_FLASH_CRC | Calculate flash CRC32 |
| 14 | RESET | Reset the microcontroller |

## Windows Application

### Download

1. Open the GitHub **Releases** section.
2. Select the latest release.
3. Download `FirmwareUpdater.exe`.
4. Launch the executable on Windows.

The standalone application is designed to run without installing Python, Visual Studio Code, or additional Python packages.

A compatible STM32 bootloader and the appropriate USB/serial drivers are required for device communication.

### Firmware Update

1. Connect the STM32 board to the computer.
2. Enter bootloader mode using the configured boot-entry method.
3. Launch `FirmwareUpdater.exe`.
4. Select USB CDC or UART.
5. Select the appropriate COM port.
6. Connect to the STM32 device.
7. Select a compatible `.bin` application firmware.
8. Verify the application flash address.
9. Start the firmware update.
10. Monitor the programming progress.
11. Wait for CRC32 verification to complete.
12. Jump to the application or reset the device.

**Important:** The uploaded firmware must be compiled for the selected microcontroller and correct application start address.

## Bootloader Configuration

The bootloader firmware can be configured through:

`user/app/bl_config.h`

Configuration options include:

- Target MCU
- UART peripheral
- UART baud rate
- UART TX and RX pins
- Bootloader entry GPIO
- Bootloader active logic level
- GPIO pull configuration

Changes to bootloader configuration require rebuilding and flashing the bootloader firmware.

## Firmware Development

The firmware source code is developed using:

- STM32Cube tools
- Visual Studio Code
- STM32 HAL and CMSIS
- CMake and Ninja
- ARM GNU Toolchain

### Building the Firmware

1. Clone or download the repository.
2. Open the repository folder in VS Code.
3. Install the STM32Cube development tools.
4. Configure the target microcontroller in `bl_config.h`.
5. Select the appropriate CMake build preset.
6. Configure and build the project.
7. Flash the generated bootloader firmware using ST-Link or another compatible programmer.

The application firmware must use the correct flash memory origin and interrupt vector table configuration.

## Project Structure

- `user/app/` — Bootloader application logic and configuration
- `user/drv/` — Communication, flash operations, command handlers, and ring buffer
- `platform/stm32f407/` — STM32F407 platform support
- `platform/stm32f746/` — STM32F746 platform support
- `cmake/` — Build system configuration
- `CMakeLists.txt` — Main CMake project
- `CMakePresets.json` — Debug and Release build presets
- `README.md` — Project documentation

## Important Notes

- The Windows desktop application communicates with a compatible bootloader; it does not install the bootloader itself.
- Flash programming modifies the application memory region.
- Interrupted firmware updates may leave the application invalid.
- Boot-entry GPIO configuration must match the hardware.
- The `.bin` application image must use the correct linker configuration.
- Validate firmware updates and reset behavior on the target board before production deployment.

## Release

**v1.0.0 — Initial Release**

Initial release of the STM32 bootloader firmware and Windows Firmware Updater application.

Includes USB CDC/UART communication, firmware programming, CRC32 verification, flash memory management, and device control.

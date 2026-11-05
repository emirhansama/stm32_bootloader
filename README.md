# Vibration Monitor

A real-time vibration monitoring and signal analysis application developed for STM32-based data acquisition systems.

The application receives three-axis acceleration data from an STM32 microcontroller over Ethernet/TCP and provides live signal visualization, frequency-domain analysis, and data recording.

## Features

- Real-time three-axis (X, Y, Z) acceleration monitoring
- Ethernet/TCP communication
- Live time-domain signal visualization
- FFT-based frequency spectrum analysis
- AC RMS acceleration calculation
- Dominant frequency detection
- Adjustable sampling frequency
- Configurable acceleration measurement range
- Adjustable FFT size and visualization settings
- FIFO overrun and dropped-sample monitoring
- CSV data recording and export
- Built-in simulation mode for testing without hardware
- Dark-themed Windows desktop interface

## Hardware

The current firmware is designed for:

- STM32F746ZG microcontroller
- IIS3DWB three-axis digital accelerometer
- Ethernet communication interface

The architecture can be adapted to compatible data acquisition hardware.

## Signal Analysis

### Time-Domain Analysis

The application visualizes acceleration measurements along the X, Y, and Z axes.

Acceleration values are displayed in gravitational units (g).

The time-domain plots allow users to observe vibration amplitude, transient changes, and signal behavior.

### Frequency-Domain Analysis

Fast Fourier Transform (FFT) is used to analyze vibration signals in the frequency domain.

The application provides:

- Frequency spectrum visualization
- Hann window processing
- Dominant frequency identification
- Configurable FFT resolution
- Three-axis frequency analysis

### AC RMS

AC RMS represents the root mean square of the acceleration signal after removing its mean value.

This measurement helps distinguish vibration from the static acceleration component associated with gravity.

## Communication

The application communicates with the STM32 device using Ethernet/TCP.

Default connection settings:

| Parameter | Default Value |
|---|---|
| Device IP | `192.168.1.50` |
| TCP Port | `5000` |
| Sampling Rate | `26667 Hz` |
| Accelerometer Range | `±16 g` |
| Data Format | Binary TCP frames |

The IP address, TCP port, sampling rate, and sensitivity can be configured in the application.

Settings must match the connected hardware and firmware configuration.

## Installation

### Windows Executable

Download `VibrationMonitor.exe` from the GitHub Releases section when a Windows build is available.

### Python Version

Requirements:

- Python 3.11 or newer (tested development range: 3.11–3.13)
- NumPy
- Matplotlib
- Windows operating system

Install the dependencies:

`py -3 -m pip install -r requirements.txt`

Run the application:

`py -3 vibration_monitor.py`

Alternatively, launch `run_monitor.bat`.

To generate a Windows executable, run `build_windows.bat`. The executable will be created in the `dist` directory.

## How to Use

1. Connect the STM32 device to the Ethernet network.
2. Ensure that the computer and STM32 device can communicate over the network.
3. Launch Vibration Monitor.
4. Enter the device IP address and TCP port.
5. Configure the sampling rate and acceleration measurement range.
6. Click Connect to start receiving data.
7. Observe live time-domain and frequency-domain plots.
8. Adjust the visualization settings as needed.
9. Start CSV recording to save measurement data.

## Simulation Mode

Vibration Monitor includes a built-in demo mode that generates synthetic acceleration signals.

This mode allows users to explore the interface and test signal visualization without connecting physical hardware.

Simulation data is clearly identified and should not be interpreted as real sensor measurements.

## CSV Data Export

Recorded acceleration data can be exported to CSV format for additional processing.

The recorded data includes:

- Sample index
- Time in seconds
- X-axis acceleration (g)
- Y-axis acceleration (g)
- Z-axis acceleration (g)

Exported measurements can be analyzed using Python, MATLAB, Excel, or other data analysis tools.

## Project Structure

- `vibration_monitor.py` — Main desktop application
- `run_monitor.bat` — Python application launcher
- `build_windows.bat` — Windows executable build script
- `requirements.txt` — Python dependencies
- `firmware/` — STM32 vibration acquisition firmware
- `tests/` — Automated software tests

## Development Environment

- Visual Studio Code
- STM32Cube tools
- STM32 HAL / CMSIS
- CMake / Ninja
- ARM GNU Toolchain
- Python
- Tkinter
- NumPy
- Matplotlib

## Testing

The application includes software tests for binary frame decoding, signal processing, and simulated data acquisition.

Physical sensor operation, Ethernet communication, and Windows executable functionality should be verified with the target hardware.

## Important Notes

Accurate vibration measurements require correct accelerometer sensitivity and sampling frequency configuration.

The desktop application and STM32 firmware must use compatible communication protocols.

FFT results depend on the sampling rate, signal quality, and selected analysis window.

## Version

**v1.0.0 — Initial Release**

Initial standalone release featuring real-time vibration monitoring, FFT-based frequency analysis, acceleration statistics, and CSV data recording.

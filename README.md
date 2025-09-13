# RTC Calibration Utility

Real-time clock calibration utility for RC2014 systems with RomWBW HBIOS RTC support.

## Features

- RTC time display and setting
- Interactive time adjustment with arrow keys
- RTC calibration against CPU clock
- Hardware testing and validation
- ANSI colour support (optional)
- HBIOS integration for maximum compatibility

## Requirements

- RC2014 with RTC hardware
- RomWBW BIOS with HBIOS RTC support
- CP/M operating system

## Building

```bash
make
```

Requires [z88dk](https://github.com/z88dk/z88dk) toolchain.

## Usage

Run `rtccalib.com` and use the interactive menu:

- **S** - Show current date/time
- **D** - Set RTC date
- **T** - Set RTC time (with arrow key adjustment)
- **H** - Hardware test
- **C** - Calibrate RTC speed
- **A** - Toggle ANSI colours
- **?** - Help
- **Q** - Quit

## Licence

This software is provided free of charge and may be freely copied, modified, and distributed. It is provided "as is" without warranty of any kind, either express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose, and non-infringement.

In no event shall the authors or copyright holders be liable for any claim, damages, or other liability, whether in an action of contract, tort, or otherwise, arising from, out of, or in connection with the software or the use or other dealings in the software.

Feel free to modify, distribute, and use this software for any purpose.

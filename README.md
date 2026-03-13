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

### Default (z88dk / RC2014 standard clock)

```bash
make
```

Requires [z88dk](https://github.com/z88dk/z88dk) toolchain.

### UC80 compiler (half-speed / 3.6864 MHz variant)

```bash
make uc80
```

Requires [UC80](https://github.com/avwohl/uc80) compiler.  Assembly files
(`rtc.asm`, `cpm.asm`) still use z88dk for `SECTION`/`PUBLIC` syntax.
Override the compiler path if needed:

```bash
make uc80 UC80=/path/to/uc80
```

Both targets accept `CPU_CLOCK_HZ` and `LOOPS_PER_SEC` overrides for custom
hardware:

```bash
make CPU_CLOCK_HZ=4000000
```

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

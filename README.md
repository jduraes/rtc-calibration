# RTC Calibration Utility for RC2014

A comprehensive RTC calibration and testing utility designed specifically for the RC2014 computer system with Ed Brindley's RTC Card Rev1.

## Features

- **Hardware Detection**: Automatically detects RP5C01 RTC chip presence
- **Time Display**: Shows current RTC time in 24-hour format
- **Time Setting**: Interactive time setting with validation
- **Calibration Test**: Measures RTC accuracy over a 10-minute period
- **Hardware Test**: Tests NVRAM functionality and basic RTC operations
- **User-friendly Menu**: Simple command-driven interface

## Hardware Requirements

- RC2014 computer system
- Ed Brindley's RTC Card Rev1 
- RP5C01 RTC chip
- RTC card configured for I/O ports B4h/B5h
- Backup battery connected and functional

## Software Requirements

- CP/M operating system on RC2014
- z88dk compiler suite (for building from source)
- Make utility (for building from source)

## Building

```bash
make
```

This will create `rtccalib.com` suitable for running on your RC2014 system.

### Build Options

- `make all` - Build the application (default)
- `make clean` - Remove build artifacts  
- `make install` - Copy to ROMWBW_APPS directory (if set)
- `make test` - Build and show usage info
- `make help` - Show build system help

## Installation

1. Build the application using `make`
2. Copy `rtccalib.com` to your RC2014 system
3. Run under CP/M: `rtccalib`

## Usage

The utility provides a menu-driven interface with the following commands:

- **T** - Display current time
- **S** - Set RTC time (interactive)
- **C** - Perform calibration test (10-minute duration)
- **H** - Hardware test (NVRAM and basic functionality)
- **?** - Show help
- **Q** - Quit program

### Calibration Test

The calibration test measures RTC accuracy by:
1. Recording start time from RTC
2. Waiting for exactly 10 minutes (using CPU timing)
3. Recording end time from RTC
4. Comparing expected vs actual elapsed time
5. Reporting accuracy percentage and deviation

Results indicate if the RTC is:
- **ACCURATE** (within 5-second tolerance)
- **SLOW** (running behind real time)
- **FAST** (running ahead of real time)

### Time Setting

When setting time, enter values in 2-digit format:
- Year: 00-99 (represents 2000-2099)
- Month: 01-12
- Day: 01-31
- Hour: 00-23 (24-hour format)
- Minute: 00-59
- Second: 00-59

## File Structure

Following 8.3 filename conventions for CP/M compatibility:

- `rtccalib.c` - Main application code
- `rp5c01.asm` - RP5C01 RTC driver assembly routines
- `rp5c01.h` - RTC driver header file
- `cpm.asm` - CP/M system interface routines
- `cpm.h` - CP/M interface header
- `Makefile` - Build configuration

## Technical Details

### RTC Interface

The RP5C01 RTC chip uses two I/O ports:
- Port B4h: Register select
- Port B5h: Data port

The driver supports:
- 24-hour time format
- BCD time/date storage
- NVRAM access (13 bytes)
- Hardware detection
- Mode control (timer/alarm/RAM modes)

### Memory Usage

The application is designed to run efficiently on 8-bit systems with limited memory:
- Compact code size suitable for CP/M TPA
- Minimal RAM usage
- No dynamic memory allocation
- Efficient BCD/decimal conversions

## Troubleshooting

### "RP5C01 RTC not detected" Error

Check the following:
1. RTC card is properly seated in RC2014 backplane
2. Card jumpers configured for I/O ports B4h/B5h  
3. Backup battery is connected and has sufficient voltage
4. No I/O port conflicts with other cards

### Calibration Issues

- Ensure system is stable during 10-minute test period
- Avoid running other programs during calibration
- Press ESC to abort test if needed
- Large deviations may indicate failing crystal or battery

### Build Issues

- Ensure z88dk is properly installed and in PATH
- Check that all source files are present
- Verify make utility is available

## License

This project is provided as-is for educational and personal use with RC2014 systems.

## Acknowledgments

- Ed Brindley for the excellent RTC card design
- RC2014 community for the platform
- RomWBW project for development inspiration

## Version History

- v1.0 - Initial release with full calibration and testing features

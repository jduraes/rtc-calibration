# RTC Calibration Utility for RC2014

A real-time clock calibration utility for RC2014 systems running RomWBW BIOS with RTC hardware support.

## Features

- **RTC Detection**: Automatically detects RTC hardware via HBIOS calls
- **Time Display**: Shows current date and time in DD/MM/YYYY HH:MM:SS format  
- **Time Setting**: User-friendly prompts for setting date and time
- **Hardware Test**: Validates RTC communication and data integrity
- **HBIOS Integration**: Uses RomWBW's HBIOS RTC services for maximum compatibility

## Version History

- **v0.1.10** - Current version with clean UI and proper HBIOS integration
- **v0.1.9** - Added user-friendly date/time input with ESC abort support
- **v0.1.8** - Fixed stack corruption bugs and improved error handling
- **v0.1.7** - Resolved HBIOS return code interpretation issues

## Requirements

- RC2014 system with RTC hardware (DS1302 or compatible)
- RomWBW BIOS with HBIOS RTC support
- CP/M compatible operating system

## Building

Requires [z88dk](https://github.com/z88dk/z88dk) cross-compiler:

```bash
make
```

This produces `rtccalib.com` for CP/M systems.

## Usage

```
rtccalib [command]

Commands:
  (none)  - Show current time
  s       - Set time  
  t       - Hardware test
  h       - Show help
```

## Technical Details

- Uses HBIOS functions 20h (get time) and 21h (set time)
- Handles both success (0x00) and valid non-zero (0xB8) return codes
- 6-byte BCD time format: YYMMDDHHMMSS
- Assembly wrappers provide proper Z80/HBIOS calling conventions

## License

Open source - feel free to modify and distribute.

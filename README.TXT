# RTC Calibration Utility for RC2014

A real-time clock calibration utility for the RC2014 system using RomWBW HBIOS RTC services.
This version provides broad compatibility with various RTC hardware through the HBIOS abstraction layer.

## Hardware Requirements

- RC2014 computer system
- Any RTC hardware supported by RomWBW HBIOS (DS1302, BQ4845P, etc.)
- RomWBW with RTC driver enabled
- Backup battery installed and connected

## Software Requirements

- RomWBW BIOS/Operating System
- z88dk toolchain for building from source
- CP/M compatible environment

## Building

```bash
make clean
make
```

This creates `rtccalib.com`.

## Usage

Copy `rtccalib.com` to your RC2014 system and run it:

```
A>rtccalib
```

### Commands

- **T** - Display current time from RTC
- **S** - Set RTC time (prompts for YY MM DD HH MM SS)
- **H** - Hardware test (displays RTC status and current time)
- **?** - Show help
- **Q** - Quit program

## Features

- BCD (Binary Coded Decimal) to decimal conversion
- Simple menu-driven interface
- Hardware detection and error handling
- No external library dependencies
- Optimized for minimal code size
- Uses RomWBW HBIOS RTC services
- Compatible with multiple RTC chip types
- Leverages RomWBW's hardware abstraction
- Robust error handling through HBIOS

## Technical Details

- Uses HBIOS function 0x20 (RTC services)
- Subfunctions 0x00 (get time) and 0x01 (set time)
- Works with HBIOS time format (YYMMDDHHMMSS BCD)
- Hardware-independent through HBIOS abstraction

## File Structure

- `rtccalib.c` - Main program source
- `rtc.asm` - HBIOS RTC interface in assembly
- `rtc.h` - Header file for HBIOS RTC functions
- `cpm.asm` - CP/M system interface
- `cpm.h` - CP/M function prototypes
- `Makefile` - Build configuration

## Troubleshooting

If "RTC not available via HBIOS":
1. Verify RomWBW has RTC support compiled in
2. Check that RTC hardware is detected by RomWBW at boot
3. Ensure RTC driver is loaded in HBIOS
4. Try the RomWBW `rtc` command to test HBIOS RTC access

If the time shows as all zeros:
1. The RTC may need initialization - use the Set command
2. Check battery voltage (should be ~3V)
3. Verify RTC crystal oscillator is running
4. Ensure RTC hardware is properly connected and configured

## Version History

- v1.0 - Initial direct hardware version for DS1302
- v2.0 - HBIOS version for broader hardware compatibility

## License

This project is provided as-is for educational and hobby use.

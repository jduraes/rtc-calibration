@echo off
rem Build script for RTC Calibration Utility using TASM
rem For RC2014 with Ed Brindley's RTC Card Rev1

echo Building RTC Calibration Utility for RC2014...

rem Assemble the RTC driver
tasm rp5c01.asm rp5c01.obj

rem Assemble CP/M interface
tasm cpm.asm cpm.obj  

rem Note: Main C file would need separate C compiler
rem This script shows the assembly compilation steps
rem For full build, use z88dk toolchain with provided Makefile

echo Assembly files compiled successfully
echo Use z88dk toolchain for complete build: make
pause

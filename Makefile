# RTC Calibration Utility Makefile
ZCC = zcc
TARGET = +cpm
# CPU clock frequency in Hz.
#   RC2014 standard: 7,372,800 Hz (default)
#   UC80 / half-speed: 3,686,400 Hz  -> make uc80
CPU_CLOCK_HZ ?= 7372800
# Timing-loop iterations per RTC second (scaled from CPU_CLOCK_HZ at build time).
# 1475 = approx cycles per loop iteration (7,372,800 Hz / 4999 loops/s, measured on RC2014).
# Override with the empirically measured value for your hardware if needed.
LOOPS_PER_SEC ?= $(shell echo $$(( $(CPU_CLOCK_HZ) / 1475 )) )
CFLAGS = -SO3 -compiler=sccz80 -DCPU_CLOCK_HZ=$(CPU_CLOCK_HZ)UL -DLOOPS_PER_SEC=$(LOOPS_PER_SEC)L
LDFLAGS = 
ASM = zcc
ASMFLAGS = +cpm
TARGET_NAME = rtccalib

# UC80 compiler (https://github.com/avwohl/uc80) — alternative to z88dk
# Override on the command line if uc80 is not on PATH: make uc80 UC80=path/to/uc80
UC80 = uc80
# Clock frequency for UC80 target (half-speed RC2014 / 3.6864 MHz variant)
UC80_CLOCK_HZ ?= 3686400
UC80_LOOPS_PER_SEC ?= $(shell echo $$(( $(UC80_CLOCK_HZ) / 1475 )) )
# UC80 uses standard C compiler flags; drop z88dk-specific -SO3 / -compiler=sccz80 / +cpm
UC80_CFLAGS = -DCPU_CLOCK_HZ=$(UC80_CLOCK_HZ)UL -DLOOPS_PER_SEC=$(UC80_LOOPS_PER_SEC)L

C_SOURCES = rtccalib.c ansi.c
ASM_SOURCES = rtc.asm cpm.asm
HEADERS = rtc.h cpm.h ansi.h

# Object files
C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

# Default target
all: $(TARGET_NAME).com
	@echo "Compiled $(TARGET_NAME).com successfully"

# Build the COM file
$(TARGET_NAME).com: $(OBJECTS)
	$(ZCC) $(TARGET) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

# Build compare test program
compare.com: compare.o $(ASM_OBJECTS)
	$(ZCC) $(TARGET) $(CFLAGS) $(LDFLAGS) -o $@ compare.o $(ASM_OBJECTS)

# Compile C source files
%.o: %.c $(HEADERS)
	$(ZCC) $(TARGET) $(CFLAGS) -c $< -o $@

# Assemble ASM source files
%.o: %.asm
	$(ASM) $(ASMFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f *.o *.com *.map *.lst
	echo "Cleaned build files"

# Install to a common location (adjust path as needed)
install: $(TARGET_NAME).com
	@if [ -n "$(ROMWBW_APPS)" ]; then \
		cp $(TARGET_NAME).com $(ROMWBW_APPS)/; \
		echo "Installed to $(ROMWBW_APPS)"; \
	else \
		echo "Set ROMWBW_APPS environment variable to install location"; \
	fi

# Test the program (requires RC2014/RomWBW environment)
test: $(TARGET_NAME).com
	@echo "Testing $(TARGET_NAME).com requires RC2014/RomWBW environment"
	@echo "Copy $(TARGET_NAME).com to your RC2014 system and run it"

# UC80 build: use UC80 compiler with half-speed clock (3.6864 MHz).
# Assembly files still assembled via z88dk (SECTION/PUBLIC syntax).
# Usage: make uc80  (or: make uc80 UC80=/path/to/uc80)
uc80:
	$(MAKE) ZCC=$(UC80) \
		TARGET="" \
		CFLAGS="$(UC80_CFLAGS)" \
		CPU_CLOCK_HZ=$(UC80_CLOCK_HZ) \
		LOOPS_PER_SEC=$(UC80_LOOPS_PER_SEC) \
		TARGET_NAME=$(TARGET_NAME)_uc80

# Display help
help:
	@echo "RTC Calibration Utility (HBIOS) - Available targets:"
	@echo "  all     - Build $(TARGET_NAME).com (default, uses z88dk/zcc)"
	@echo "  uc80    - Build $(TARGET_NAME)_uc80.com using UC80 compiler"
	@echo "  clean   - Remove build artifacts"
	@echo "  install - Copy program to ROMWBW_APPS/"
	@echo "  test    - Show testing instructions"
	@echo "  help    - Show this help"
	@echo ""
	@echo "Requirements:"
	@echo "  all  : z88dk toolchain (https://github.com/z88dk/z88dk)"
	@echo "  uc80 : UC80 compiler   (https://github.com/avwohl/uc80)"
	@echo "         Assembly objects still built with z88dk for SECTION/PUBLIC syntax"
	@echo ""
	@echo "Clock speed targets:"
	@echo "  all      - Build for RC2014 standard (CPU_CLOCK_HZ=$(CPU_CLOCK_HZ))"
	@echo "  uc80     - Build for UC80 half-speed  (CPU_CLOCK_HZ=$(UC80_CLOCK_HZ))"
	@echo "  Override: make CPU_CLOCK_HZ=<hz> [LOOPS_PER_SEC=<n>]"

.PHONY: all clean install test help uc80

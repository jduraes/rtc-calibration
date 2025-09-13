# RTC Calibration Utility Makefile
ZCC = zcc
TARGET = +cpm
CFLAGS = -SO3 -compiler=sccz80
LDFLAGS = 
ASM = zcc
ASMFLAGS = +cpm
TARGET_NAME = rtccalib

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

# Display help
help:
	@echo "RTC Calibration Utility (HBIOS) - Available targets:"
	@echo "  all     - Build $(TARGET_NAME).com (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  install - Copy program to ROMWBW_APPS/"
	@echo "  test    - Show testing instructions"
	@echo "  help    - Show this help"
	@echo ""
	@echo "Requirements:"
	@echo "  - z88dk toolchain"
	@echo "  - RC2014 with RomWBW HBIOS"
	@echo "  - RTC hardware supported by RomWBW"

.PHONY: all clean install test help

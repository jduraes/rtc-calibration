	PUBLIC	_ds1302_init, _ds1302_detect, _ds1302_get_time, _ds1302_set_time

	SECTION code_user

; DS1302 RTC I/O port definitions for Ed Brindley's RTC card
; These need to be verified against the actual card schematic
DS1302_DATA	EQU	$B0		; Data I/O port
DS1302_CLK	EQU	$B1		; Clock output port  
DS1302_RST	EQU	$B2		; Reset/CE output port

; DS1302 Register addresses (with command bits)
DS1302_SEC_WR	EQU	$80		; Seconds write
DS1302_SEC_RD	EQU	$81		; Seconds read
DS1302_MIN_WR	EQU	$82		; Minutes write
DS1302_MIN_RD	EQU	$83		; Minutes read
DS1302_HR_WR	EQU	$84		; Hours write
DS1302_HR_RD	EQU	$85		; Hours read
DS1302_DATE_WR	EQU	$86		; Date write
DS1302_DATE_RD	EQU	$87		; Date read
DS1302_MON_WR	EQU	$88		; Month write
DS1302_MON_RD	EQU	$89		; Month read
DS1302_YEAR_WR	EQU	$8C		; Year write
DS1302_YEAR_RD	EQU	$8D		; Year read
DS1302_WP_WR	EQU	$8E		; Write protect write
DS1302_WP_RD	EQU	$8F		; Write protect read

; Clock halt bit in seconds register
DS1302_CH_BIT	EQU	$80		; Clock halt bit

;
; Initialize DS1302 RTC
; void ds1302_init(void)
;
_ds1302_init:
	; Set RST and CLK low initially
	XOR	A
	LD	C, DS1302_RST
	OUT	(C), A
	LD	C, DS1302_CLK
	OUT	(C), A
	
	; Disable write protection
	LD	A, DS1302_WP_WR
	LD	B, 0			; Clear write protect
	CALL	DS1302_WRITE_REG
	
	; Enable oscillator (clear CH bit in seconds register)
	LD	A, DS1302_SEC_RD
	CALL	DS1302_READ_REG
	AND	$7F			; Clear CH bit (bit 7)
	LD	B, A
	LD	A, DS1302_SEC_WR
	CALL	DS1302_WRITE_REG
	
	RET

;
; Detect DS1302 presence
; uint8_t ds1302_detect(void)
; Returns: 1 if detected, 0 if not
;
_ds1302_detect:
	; Try to read seconds register
	LD	A, DS1302_SEC_RD
	CALL	DS1302_READ_REG
	
	; Save original value
	LD	C, A
	
	; Write a test pattern (0x55)
	LD	A, DS1302_WP_WR
	LD	B, 0			; Disable write protect first
	CALL	DS1302_WRITE_REG
	
	LD	A, DS1302_SEC_WR
	LD	B, $55			; Test pattern
	CALL	DS1302_WRITE_REG
	
	; Read back
	LD	A, DS1302_SEC_RD
	CALL	DS1302_READ_REG
	
	; Check if we got back our test pattern (minus CH bit)
	AND	$7F			; Mask CH bit
	CP	$55
	JR	Z, DS1302_DETECTED
	
	; Not detected
	LD	L, 0
	JR	DS1302_DETECT_END
	
DS1302_DETECTED:
	; Restore original value
	LD	A, DS1302_SEC_WR
	LD	B, C
	CALL	DS1302_WRITE_REG
	
	; Enable write protect
	LD	A, DS1302_WP_WR
	LD	B, $80
	CALL	DS1302_WRITE_REG
	
	LD	L, 1

DS1302_DETECT_END:
	RET

;
; Get time from DS1302
; void ds1302_get_time(DS1302_Time *time) __z88dk_fastcall
; HL points to DS1302_Time structure
;
_ds1302_get_time:
	PUSH	HL			; Save structure pointer
	
	; Read seconds
	LD	A, DS1302_SEC_RD
	CALL	DS1302_READ_REG
	AND	$7F			; Clear CH bit
	LD	(HL), A			; Store seconds
	INC	HL
	
	; Read minutes  
	LD	A, DS1302_MIN_RD
	CALL	DS1302_READ_REG
	LD	(HL), A			; Store minutes
	INC	HL
	
	; Read hours
	LD	A, DS1302_HR_RD
	CALL	DS1302_READ_REG
	AND	$3F			; Clear 12/24 and AM/PM bits (assume 24h)
	LD	(HL), A			; Store hours
	INC	HL
	
	; Read date
	LD	A, DS1302_DATE_RD
	CALL	DS1302_READ_REG
	LD	(HL), A			; Store date
	INC	HL
	
	; Read month
	LD	A, DS1302_MON_RD
	CALL	DS1302_READ_REG
	LD	(HL), A			; Store month
	INC	HL
	
	; Read year
	LD	A, DS1302_YEAR_RD
	CALL	DS1302_READ_REG
	LD	(HL), A			; Store year
	
	POP	HL			; Restore structure pointer
	RET

;
; Set time to DS1302
; void ds1302_set_time(const DS1302_Time *time) __z88dk_fastcall
; HL points to DS1302_Time structure
;
_ds1302_set_time:
	PUSH	HL			; Save structure pointer
	
	; Disable write protection
	LD	A, DS1302_WP_WR
	LD	B, 0
	CALL	DS1302_WRITE_REG
	
	POP	HL			; Restore structure pointer
	PUSH	HL			; Save again
	
	; Write seconds (with CH=0 to enable oscillator)
	LD	A, DS1302_SEC_WR
	LD	B, (HL)
	RES	7, B			; Clear CH bit to enable oscillator
	CALL	DS1302_WRITE_REG
	INC	HL
	
	; Write minutes
	LD	A, DS1302_MIN_WR
	LD	B, (HL)
	CALL	DS1302_WRITE_REG
	INC	HL
	
	; Write hours (24-hour format)
	LD	A, DS1302_HR_WR
	LD	B, (HL)
	RES	7, B			; Ensure 24-hour format
	CALL	DS1302_WRITE_REG
	INC	HL
	
	; Write date
	LD	A, DS1302_DATE_WR
	LD	B, (HL)
	CALL	DS1302_WRITE_REG
	INC	HL
	
	; Write month
	LD	A, DS1302_MON_WR
	LD	B, (HL)
	CALL	DS1302_WRITE_REG
	INC	HL
	
	; Write year
	LD	A, DS1302_YEAR_WR
	LD	B, (HL)
	CALL	DS1302_WRITE_REG
	
	; Re-enable write protection
	LD	A, DS1302_WP_WR
	LD	B, $80
	CALL	DS1302_WRITE_REG
	
	POP	HL			; Restore structure pointer
	RET

;
; Low-level DS1302 register read
; Input: A = register address (with read bit set)
; Output: A = data read
;
DS1302_READ_REG:
	PUSH	BC
	
	; Start transmission - set RST high
	LD	B, 1
	LD	C, DS1302_RST
	OUT	(C), B
	
	; Small delay
	CALL	DS1302_DELAY
	
	; Send command byte
	CALL	DS1302_WRITE_BYTE
	
	; Read data byte
	CALL	DS1302_READ_BYTE
	
	; End transmission - set RST low
	XOR	A
	LD	C, DS1302_RST
	OUT	(C), A
	LD	C, DS1302_CLK
	OUT	(C), A
	
	POP	BC
	RET

;
; Low-level DS1302 register write
; Input: A = register address (with write bit set)
;        B = data to write
;
DS1302_WRITE_REG:
	PUSH	BC
	
	; Start transmission - set RST high
	LD	A, 1
	LD	C, DS1302_RST
	OUT	(C), A
	
	; Small delay
	CALL	DS1302_DELAY
	
	; Send command byte
	CALL	DS1302_WRITE_BYTE
	
	; Send data byte
	LD	A, B
	CALL	DS1302_WRITE_BYTE
	
	; End transmission - set RST low
	XOR	A
	LD	C, DS1302_RST
	OUT	(C), A
	LD	C, DS1302_CLK
	OUT	(C), A
	
	POP	BC
	RET

;
; Write byte to DS1302 (LSB first)
; Input: A = byte to write
;
DS1302_WRITE_BYTE:
	PUSH	BC
	LD	B, 8			; 8 bits to send
	LD	C, A			; Save byte to send
	
DS1302_WR_LOOP:
	; Set data line according to bit 0 of C
	LD	A, C
	AND	1
	PUSH	BC
	LD	C, DS1302_DATA
	OUT	(C), A
	POP	BC
	
	; Clock high
	LD	A, 1
	PUSH	BC
	LD	C, DS1302_CLK
	OUT	(C), A
	POP	BC
	CALL	DS1302_DELAY
	
	; Clock low
	XOR	A
	PUSH	BC
	LD	C, DS1302_CLK
	OUT	(C), A
	POP	BC
	CALL	DS1302_DELAY
	
	; Shift to next bit
	SRL	C
	
	DJNZ	DS1302_WR_LOOP
	
	POP	BC
	RET

;
; Read byte from DS1302 (LSB first)
; Output: A = byte read
;
DS1302_READ_BYTE:
	PUSH	BC
	PUSH	DE
	LD	B, 8			; 8 bits to read
	LD	D, 0			; Accumulate result here
	
DS1302_RD_LOOP:
	; Clock high
	LD	A, 1
	LD	C, DS1302_CLK
	OUT	(C), A
	CALL	DS1302_DELAY
	
	; Read data bit
	LD	C, DS1302_DATA
	IN	A, (C)
	AND	1			; Mask to bit 0
	
	; Shift into result (LSB first)
	SRL	D			; Shift result right
	OR	A			; Test data bit
	JR	Z, DS1302_RD_SKIP	; If bit was 0, skip
	SET	7, D			; Set bit 7 if data bit was 1
	
DS1302_RD_SKIP:
	; Clock low
	XOR	A
	LD	C, DS1302_CLK
	OUT	(C), A
	CALL	DS1302_DELAY
	
	DJNZ	DS1302_RD_LOOP
	
	LD	A, D			; Return result
	POP	DE
	POP	BC
	RET

;
; Small delay routine
;
DS1302_DELAY:
	PUSH	BC
	LD	B, 10			; Adjust as needed for timing
DS1302_DLY_LOOP:
	NOP
	DJNZ	DS1302_DLY_LOOP
	POP	BC
	RET

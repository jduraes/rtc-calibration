; Ultra-safe RTC test program - exact copy of working rtchb.asm logic
; This should not crash the system

	ORG	0100H

MAIN:
	; Save original stack pointer
	LD	(ORIG_SP), SP
	LD	SP, LOCAL_STACK

	; Print startup message
	LD	DE, START_MSG
	LD	C, 9
	CALL	5

	; EXACT copy of rtchb.asm RTC call
	LD	B, 20H		; RTC function (exactly like rtchb.asm)
	LD	HL, TIME_BUF	; Buffer for time data
	RST	08		; Call HBIOS

	; Check result
	OR	A
	JR	NZ, ERROR_EXIT

	; Success - print the result
	LD	DE, SUCCESS_MSG
	LD	C, 9
	CALL	5

	; Print raw buffer contents
	LD	HL, TIME_BUF
	LD	B, 6		; 6 bytes
PRINT_LOOP:
	PUSH	BC
	PUSH	HL
	LD	A, (HL)
	CALL	PRINT_HEX
	LD	A, ' '
	LD	E, A
	LD	C, 2
	CALL	5
	POP	HL
	POP	BC
	INC	HL
	DJNZ	PRINT_LOOP

	JR	NORMAL_EXIT

ERROR_EXIT:
	; Print error code
	LD	DE, ERROR_MSG
	LD	C, 9
	CALL	5
	CALL	PRINT_HEX

NORMAL_EXIT:
	; Print newline
	LD	DE, NEWLINE
	LD	C, 9
	CALL	5

	; Restore original stack
	LD	SP, (ORIG_SP)

	; Exit to CP/M
	LD	C, 0
	CALL	5

; Print hex byte in A
PRINT_HEX:
	PUSH	AF
	RRC	A
	RRC	A
	RRC	A
	RRC	A
	AND	0FH
	CALL	PRINT_NIBBLE
	POP	AF
	AND	0FH
	CALL	PRINT_NIBBLE
	RET

PRINT_NIBBLE:
	CP	10
	JR	C, NUMERIC
	ADD	A, 'A' - 10
	JR	PRINT_IT
NUMERIC:
	ADD	A, '0'
PRINT_IT:
	LD	E, A
	LD	C, 2
	CALL	5
	RET

; Data area
START_MSG:	DB	'Safe RTC Test v1.0', 13, 10, '$'
SUCCESS_MSG:	DB	'RTC Success: ', '$'
ERROR_MSG:	DB	'RTC Error: ', '$'
NEWLINE:	DB	13, 10, '$'

; Buffer for RTC data (6 bytes)
TIME_BUF:	DS	6

; Stack area
		DS	64	; 64 byte local stack
LOCAL_STACK:

; Original stack pointer storage
ORIG_SP:	DW	0

	END

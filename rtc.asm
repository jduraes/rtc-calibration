	PUBLIC	_hbios_rtc_detect, _hbios_rtc_get_time, _hbios_rtc_set_time, _hbios_rtc_test

	SECTION code_user

; HBIOS RTC function constants
BF_RTC		EQU	20h		; RTC get time function
BF_RTCSET	EQU	21h		; RTC set time function


;
; Detect RTC presence by attempting to get time
; int hbios_rtc_detect(void)
; Returns: 1 if RTC detected, 0 if not
;
_hbios_rtc_detect:
	; BC and DE are caller-save in sccz80 calling convention; no push needed.
	; Try to get time to test RTC presence (with unit 0)
	LD	B, 20h			; HBIOS RTC get time function
	LD	D, 0			; Unit 0
	LD	HL, TIME_BUF_DETECT	; Point to dedicated detect buffer
	RST	08			; Call HBIOS via RST
	
	; Check result - A contains error code (0 = success)
	OR	A			; Test A for zero
	JR	Z, _detect_success	; Jump if successful (A=0)
	
	; RTC not available or error
	LD	HL, 0			; Return 0 (not detected)
	RET
	
_detect_success:
	LD	HL, 1			; Return 1 (detected)
	RET

;
; Get time from HBIOS RTC
; int hbios_rtc_get_time(RTC_Time *time)
; HL points to RTC_Time structure
; Returns: 0 on success, -1 on error
;
_hbios_rtc_get_time:
	PUSH	HL			; Save structure pointer

	; Call HBIOS to read RTC time
	LD	B, 20h			; HBIOS RTC get time function
	LD	HL, TIME_BUF_GET	; Point to dedicated get buffer
	LD	D, 0			; Unit 0
	RST	08			; Call HBIOS via RST
	
	; Save the HBIOS return code in C (BC/DE are caller-save; C used as scratch)
	LD	C, A			; Save return code in C
	
	; Get structure pointer back
	POP	HL			; HL now points to structure
	
	; Copy HBIOS data directly to structure
	; HBIOS: [0]=YY [1]=MM [2]=DD [3]=HH [4]=MM [5]=SS
	; Our:   [0]=SS [1]=MM [2]=HH [3]=DD [4]=MM [5]=YY
	
	; Copy seconds (HBIOS byte 5 -> our byte 0)
	LD	A, (TIME_BUF_GET+5)
	LD	(HL), A
	INC	HL
	
	; Copy minutes (HBIOS byte 4 -> our byte 1)
	LD	A, (TIME_BUF_GET+4)
	LD	(HL), A
	INC	HL
	
	; Copy hours (HBIOS byte 3 -> our byte 2)
	LD	A, (TIME_BUF_GET+3)
	LD	(HL), A
	INC	HL
	
	; Copy date (HBIOS byte 2 -> our byte 3)
	LD	A, (TIME_BUF_GET+2)
	LD	(HL), A
	INC	HL
	
	; Copy month (HBIOS byte 1 -> our byte 4)
	LD	A, (TIME_BUF_GET+1)
	LD	(HL), A
	INC	HL
	
	; Copy year (HBIOS byte 0 -> our byte 5)
	LD	A, (TIME_BUF_GET+0)
	LD	(HL), A
	
	; Return the original HBIOS code for debugging
	LD	L, C			; Return HBIOS code from C
	LD	H, 0			; Clear high byte
	
_get_time_exit:
	RET

;
; Set time to HBIOS RTC
; int hbios_rtc_set_time(const RTC_Time *time)
; HL points to RTC_Time structure
; Returns: 0 on success, -1 on error
;
_hbios_rtc_set_time:
	; BC and DE are caller-save in sccz80 calling convention; no push needed.

	; Reorder bytes from our format to HBIOS format in one linear pass.
	; Our:   [0]=SS [1]=MM [2]=HH [3]=DD [4]=MM [5]=YY
	; HBIOS: [0]=YY [1]=MM [2]=DD [3]=HH [4]=MM [5]=SS

	; Seconds (our byte 0 -> HBIOS byte 5)
	LD	A, (HL)
	LD	(TIME_BUF_SET+5), A
	INC	HL

	; Minutes (our byte 1 -> HBIOS byte 4)
	LD	A, (HL)
	LD	(TIME_BUF_SET+4), A
	INC	HL

	; Hours (our byte 2 -> HBIOS byte 3)
	LD	A, (HL)
	LD	(TIME_BUF_SET+3), A
	INC	HL

	; Date (our byte 3 -> HBIOS byte 2)
	LD	A, (HL)
	LD	(TIME_BUF_SET+2), A
	INC	HL

	; Month (our byte 4 -> HBIOS byte 1)
	LD	A, (HL)
	LD	(TIME_BUF_SET+1), A
	INC	HL

	; Year (our byte 5 -> HBIOS byte 0)
	LD	A, (HL)
	LD	(TIME_BUF_SET+0), A

	; Call HBIOS to set time
	LD	B, 21h			; HBIOS RTC set time function
	LD	D, 0			; Unit 0
	LD	HL, TIME_BUF_SET	; Point to HBIOS buffer
	RST	08			; Call HBIOS via RST

	; Return 0 on success, 0xFFFF on error
	OR	A			; Test A for zero
	LD	HL, 0			; Assume success
	JR	Z, _set_time_exit	; Jump if successful
	LD	HL, 0FFFFh		; Return -1 (error)

_set_time_exit:
	RET

;
; Simple test function exactly like working example
; int hbios_rtc_test(void)
; Returns error code directly from HBIOS
;
_hbios_rtc_test:
	; DE is caller-save in sccz80 calling convention; no push needed.
	; Try with unit 0 first (most common)
	LD	B, 20h			; RTC get time function
	LD	D, 0			; Unit 0
	LD	HL, TIME_BUF		; Point to buffer
	RST	08			; Call HBIOS
	
	; Return error code in HL
	LD	L, A			; Return HBIOS error code
	LD	H, 0			; Clear high byte
	
	RET

	SECTION data_user

; Separate buffers for each function to prevent corruption
TIME_BUF_DETECT:	DS	6	; Buffer for detect function
TIME_BUF_GET:		DS	6	; Buffer for get_time function  
TIME_BUF_SET:		DS	6	; Buffer for set_time function
TIME_BUF_TEST:		DS	6	; Buffer for test function

; Legacy buffer name for compatibility
TIME_BUF:		EQU	TIME_BUF_TEST

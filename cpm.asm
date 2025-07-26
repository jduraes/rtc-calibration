	PUBLIC	_cRawIo

	SECTION code_user

; char cRawIo(void)
; check for keypress and return, otherwise 0
_cRawIo:
	LD	C, 6
	LD	E, $FF
	CALL	5
	LD	L, A
	RET

.386					; target processor: pentium
.model flat, c			; memory model must be FLAT on 32 bit, langtype can be C or STDCALL
option casemap :none	; case sensitive
; .stack					; define a stack segment, size defaults to 1024

; .data					; start of near data segment for initialized data called _DATA

.code					; start of code segment

externdef g_global : dword	; declare extern variable

getGlobal proc
	mov eax, g_global
	ret
getGlobal endp

end						; marks end of file, optionally links entry point

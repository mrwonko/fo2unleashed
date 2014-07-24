.386					; target processor: pentium
.model flat, c			; memory model must be FLAT on 32 bit, langtype can be C or STDCALL
option casemap :none	; case sensitive
; .stack					; define a stack segment, size defaults to 1024

; .data					; start of near data segment for initialized data called _DATA

.code					; start of code segment


;    extern variables

extern FO2_detail_mountFromFileList : dword

;    functions

; mountFromFileList( const char* filename )
;
; mounting the bfs files listed in a file
; filename is supplied via ESI in FO2 function
FO2_mountFromFileList proc filename : dword
	push esi
	mov esi, filename
	mov eax, dword ptr [FO2_detail_mountFromFileList]
	test eax, eax
	jz @F ; following anonymous label
	call eax
@@:
	pop esi
	ret
FO2_mountFromFileList endp

end						; marks end of file, optionally links entry point

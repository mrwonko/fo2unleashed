.386					; target processor: pentium
.model flat, c			; memory model must be FLAT on 32 bit, langtype can be C or STDCALL
option casemap : none	; case sensitive
; .stack					; define a stack segment, size defaults to 1024

; .data					; start of near data segment for initialized data called _DATA

.code					; start of code segment

;    extern variables
include addresses.inc

;    extern hooks
include hooks.inc
 
;    Hooks
;    (Jumped into from FlatOut 2 code, call C-hooks)

; mount
; jumped into after mounting files listed in "filesystem" 
; and setting esi = "patch" instead of calling
; FO2_detail_mountFromFileList
FO2_bridge_mount proc
	call [FO2_detail_mountFromFileList] ; original instruction
	invoke hook_mount
	jmp [FO2_detail_mountReturn]
FO2_bridge_mount endp

end						; marks end of file, optionally links entry point

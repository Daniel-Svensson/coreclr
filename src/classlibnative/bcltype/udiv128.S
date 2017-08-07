
; extern "C" digit64 _udiv128(digit64 low, digit64 hi, digit64 divisor, digit64 *remainder);

; Arguments
; RCX       Low Digit
; RDX       High Digit
; R8        Divisor
; R9        *Remainder

; RAX       Quotient upon return

.code

DivMod64By32 proc
    mov rax, rcx    ; Put the low digit in place (hi is already there)
    div r8d         ; 64 bit divide edx:eax/r8d = edx remainder, eax quotient
    mov [r9], edx   ; Save the reminder
    ret     ; Return the quotient, already in rax
DivMod64By32 endp

DivMod128By64 proc
    mov rax, rcx    ; Put the low digit in place (hi is already there)
    div r8      ; 128 bit divide rdx:rax/r8 = rdx remainder, rax quotient
    mov [r9], rdx   ; Save the reminder
    ret     ; Return the quotient, already in rax
DivMod128By64 endp

; Arguments
; RCX       Low Digit
; RDX       High Digit
; R8        Divisor

; RAX       Quotient upon return
; extern "C" digit64 _udiv128_v2(digit64 *low, digit64 hi, digit64 divisor);

DivMod64By32InPlace proc
	mov eax, [rcx]    ; Put the low digit in place (hi is already there)
    div r8d      ; 128 bit divide rdx:rax/r8 = rdx remainder, rax quotient
    mov [rcx], eax   ; Save the quotient
	mov rax, rdx	; Set remainder as return value
    ret     ; Return the remainder
DivMod64By32InPlace endp

DivMod128By64InPlace proc
    mov rax, [rcx]    ; Put the low digit in place (hi is already there)
    div r8      ; 128 bit divide rdx:rax/r8 = rdx remainder, rax quotient
    mov [rcx], rax   ; Save the quotient
	mov rax, rdx	; Set remainder as return value
    ret     ; Return the remainder
DivMod128By64InPlace endp

end
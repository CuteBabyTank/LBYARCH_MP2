default rel

section .text
global daxpy_asm

; void daxpy_asm(double *z, const double *x, const double *y, const double *pA, size_t n);
; RCX = z
; RDX = x
; R8  = y
; R9  = pA
; n = [rsp + 40]

daxpy_asm:
    mov     r10, [rsp + 40]     ; r10 = n
    test    r10, r10
    jle     .done

    movsd   xmm0, [r9]          ; xmm0 = *pA (A)

.loop:
    movsd   xmm1, [rdx]         ; X[i]
    mulsd   xmm1, xmm0          ; A * X[i]
    addsd   xmm1, [r8]          ; + Y[i]
    movsd   [rcx], xmm1         ; store into Z[i]

    add     rcx, 8
    add     rdx, 8
    add     r8,  8

    dec     r10
    jg      .loop

.done:
    ret

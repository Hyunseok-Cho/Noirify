.intel_syntax noprefix

.globl _set_rgb
.globl _to_grayscale
.globl set_rgb
.globl to_grayscale

.text

_set_rgb:
set_rgb:
    ret

# void to_grayscale(uint8_t* buffer, int width, int height)
_to_grayscale:
to_grayscale:
    push rbx
    push r12
    push r13
    push r14

#if defined(__APPLE__) || defined(__linux__)
    mov r10, rdi        # buffer pointer (ARG1)
    mov r11, rsi        # width          (ARG2)
    movsxd r12, edx     # height         (ARG3_32)
#else
    mov r10, rcx        # buffer pointer (ARG1)
    mov r11, rdx        # width          (ARG2)
    movsxd r12, r8d     # height         (ARG3_32)
#endif

    mov rax, r11
    imul rax, r12
    mov rcx, rax

    mov rsi, r10        # source pointer
    mov rdi, r10        # dest pointer

    mov r8d, 77
    mov r9d, 150
    mov r11d, 29

loop_pixels:
    test rcx, rcx
    jz done

    movzx eax, byte ptr [rsi + 0]  # R
    imul eax, r8d
    mov r13d, eax

    movzx eax, byte ptr [rsi + 1]  # G
    imul eax, r9d
    add r13d, eax

    movzx eax, byte ptr [rsi + 2]  # B
    imul eax, r11d
    add eax, r13d

    shr eax, 8                     # / 256

    mov byte ptr [rdi + 0], al
    mov byte ptr [rdi + 1], al
    mov byte ptr [rdi + 2], al

    add rsi, 4
    add rdi, 4
    dec rcx
    jmp loop_pixels

done:
    pop r14
    pop r13
    pop r12
    pop rbx
    ret
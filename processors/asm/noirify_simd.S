.intel_syntax noprefix
.global set_rgb
.global to_grayscale

.text

# void set_rgb(uint8_t r, uint8_t g, uint8_t b)
set_rgb:
    ret

# void to_grayscale(uint8_t* buffer, int width, int height)
# Args: buffer = rcx, width = rdx, height = r8d
to_grayscale:
    push rbx
    push r12
    push r13
    push r14
    sub rsp, 8

    mov r10, rcx        # buffer pointer
    mov r11, rdx        # width
    movsxd r12, r8d     # height -> 64-bit
    mov rdx, r11
    imul rdx, r12       # total pixels

    mov rsi, r10        # source pointer
    mov rdi, r10        # dest pointer

    # Constant RGB in registry (.data was broken)
    mov ecx, 77      # RED
    mov r9d, 150     # GREEN
    mov r11d, 29     # BLUE

loop_pixels:
    cmp rdx, 0
    je done

    movzx eax, byte ptr [rsi + 0]  # R
    imul eax, ecx                  # R*RED
    mov r8d, eax

    movzx eax, byte ptr [rsi + 1]  # G
    imul eax, r9d                   # G*GREEN
    add r8d, eax

    movzx eax, byte ptr [rsi + 2]  # B
    imul eax, r11d                  # B*BLUE
    add eax, r8d

    shr eax, 8                      # divide by 256

    # Write grayscale back
    mov byte ptr [rdi + 0], al
    mov byte ptr [rdi + 1], al
    mov byte ptr [rdi + 2], al
    # leave alpha at [rdi + 3] unchanged

    add rsi, 4
    add rdi, 4
    dec rdx
    jmp loop_pixels

done:
    add rsp, 8
    pop r14
    pop r13
    pop r12
    pop rbx
    ret

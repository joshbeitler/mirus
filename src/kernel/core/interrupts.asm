extern isr_handler

; Macro for ISRs without error codes
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push 0                  ; Push dummy error code
    push %1                 ; Push interrupt number
    jmp isr_common_stub
%endmacro

; Macro for ISRs with error codes
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    push %1                 ; Push interrupt number
    jmp isr_common_stub
%endmacro

; Define ISRs
ISR_NOERRCODE 0  ; Division by zero
ISR_NOERRCODE 1  ; Debug
ISR_NOERRCODE 2  ; Non-maskable interrupt
ISR_NOERRCODE 3  ; Breakpoint
ISR_NOERRCODE 4  ; Overflow (INTO)
ISR_NOERRCODE 5  ; Bound range exceeded
ISR_NOERRCODE 6  ; Invalid opcode
ISR_NOERRCODE 7  ; Device not available
ISR_ERRCODE   8  ; Double fault
ISR_NOERRCODE 9  ; Coprocessor segment overrun (reserved)
ISR_ERRCODE   10 ; Invalid TSS
ISR_ERRCODE   11 ; Segment not present
ISR_ERRCODE   12 ; Stack-segment fault
ISR_ERRCODE   13 ; General protection fault
ISR_ERRCODE   14 ; Page fault
ISR_NOERRCODE 15 ; Reserved
ISR_NOERRCODE 16 ; x87 Floating-point exception
ISR_ERRCODE   17 ; Alignment check
ISR_NOERRCODE 18 ; Machine check
ISR_NOERRCODE 19 ; SIMD Floating-point exception
ISR_NOERRCODE 20 ; Virtualization exception
ISR_NOERRCODE 21 ; Control protection exception
ISR_NOERRCODE 22 ; Reserved
ISR_NOERRCODE 23 ; Reserved
ISR_NOERRCODE 24 ; Reserved
ISR_NOERRCODE 25 ; Reserved
ISR_NOERRCODE 26 ; Reserved
ISR_NOERRCODE 27 ; Reserved
ISR_NOERRCODE 28 ; Reserved
ISR_NOERRCODE 29 ; Reserved
ISR_NOERRCODE 30 ; Reserved
ISR_NOERRCODE 31 ; Reserved

; Common ISR stub
isr_common_stub:
    ; Save all registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Call C handler
    mov rdi, rsp        ; Pointer to stack as first argument
    mov rsi, [rsp + 120] ; Interrupt number as second argument (15 registers * 8 bytes = 120)
    call isr_handler

    ; Restore all registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Clean up stack
    add rsp, 16     ; Remove error code and interrupt number

    ; Return from interrupt
    iretq

; Export symbols
global isr_common_stub

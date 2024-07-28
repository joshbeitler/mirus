bits 64
section .text
global load_gdt
load_gdt:
    lgdt [rdi]   ; Load the GDTR using the address passed in RDI
    ret
global reload_segments
reload_segments:
    ; Update the Code Segment
    ; We perform a far jump to flush the CPU pipeline and update CS register
    lea rax, [rel flush_cs]   ; Load the address of the label to jump to
    push 0x08                 ; Push the new code segment selector (example: 0x08)
    push rax                  ; Push the offset to jump to
    retfq                     ; Return far, which performs the jump

flush_cs:
    ; Update Data Segment Registers
    mov ax, 0x10              ; Load the new data segment selector (example: 0x10)
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ; Optional: Set up the stack pointer if needed (e.g., for new SS)
    ; mov rsp, [some_kernel_stack_address]

    ; Jump to the next piece of code in the new code segment
    jmp next_label_in_code    ; Adjust this to where you need to continue execution

next_label_in_code:
    ; The code continues executing here in the new code segment.

bits 64
section .text

global idt_load
idt_load:
  lidt [rdi]   ; Load the IDTR using the address passed in RDI
  ret

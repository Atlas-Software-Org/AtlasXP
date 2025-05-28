;   Author: Adam Bassem
;   Revision 0
;   Patch 0
;   Minor 0
;   Major 0
;   Atlas 0.0.7

[bits 64]

section .text
global KiGdtLoad

KiGdtLoad:   
    lgdt [rdi]
    mov ax, 0x10 
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    pop rdi
    mov rax, 0x08
    push rax
    push rdi
    retfq
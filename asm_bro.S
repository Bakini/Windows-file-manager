.intel_syntax noprefix
.text
.globl addAsm
addAsm:
    mov rax, rcx
    add rax, rdx
    ret
